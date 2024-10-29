#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#define __USE_XOPEN
#include <time.h>
#include "mysql.h"
#include "mysql.c"
#include "commun.c"
#include "password.h"
#include "totp.c"
#include "crypto.c"

int main()
{
char *envoi;
int i,start,success;
char *query,*reponse,*reponse2,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*owner,*mdpsave;

envoi=read_POST();
success=0;
start=strlen(envoi);
for(i=0;i<start;i++)
	{
	if(envoi[i]<32) envoi[i]=0;
	}
query_size=atoi(getenv("CONTENT_LENGTH"));
nom=(char*)malloc(1000+query_size);
mdp=(char*)malloc(1000+query_size);
mdpsave=(char*)malloc(1000+query_size);
code=(char*)malloc(1000+query_size);
owner=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
reponse2=(char*)malloc(20000);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
if(veille_au_grain4(envoi,4,nom,mdp,code,owner,0)==0)
	{
	get_chaine(envoi,4,nom);
	tamb(nom);
	get_chaine(envoi,3,mdp);
	tamb(mdp);
	get_chaine(envoi,2,code);
	tamb(code);
	get_chaine(envoi,1,owner);
	tamb(owner);
	}
strcpy(mdpsave,mdp);
hache(mdp);
success=0;
if((handler=db_opendatabase("reflex","localhost","reflex",PASSWORD))==NULL)
	{
	strcpy(reponse,"Erreur : Impossible d'accéder à la base de données");
	strcpy(lienretour,"/reflex/index.html");
	strcpy(textebouton,"Réessayer");
	}
else
	{
	sprintf(query,"select * from compte where email=\'%s\';",nom);
	db_query(handler,query);
	if(db_ntuples(result)==0)
		{
		strcpy(reponse,"Erreur : Ce compte n'existe pas ...");
		strcpy(lienretour,"/reflex/index.html");
		strcpy(textebouton,"Réessayer");
		db_clear_result(result);
		db_close(handler);
		}
	else
		{
		db_clear_result(result);
		sprintf(query,"select * from compte where email=\'%s\' and mdp=\'%s\';",nom,mdp);
		db_query(handler,query);
		if(db_ntuples(result)==0)
			{
			strcpy(reponse,"Erreur : Mauvais mot de passe ...");
			strcpy(lienretour,"/reflex/index.html");
			strcpy(textebouton,"Réessayer");
			db_clear_result(result);
			db_close(handler);
			sleep(2);
			}
		else
			{
			db_clear_result(result);
			sprintf(query,"select * from compte where email=\'%s\' and mdp=\'%s\' and actif=\'oui\';",nom,mdp);
			db_query(handler,query);
			if(db_ntuples(result)==0)
				{
				strcpy(reponse,"Erreur : Votre compte n'a pas été activé par l'administrateur ...");
				strcpy(lienretour,"/reflex/index.html");
				strcpy(textebouton,"Réessayer");
				db_clear_result(result);
				db_close(handler);
				}
			else
				{
				db_clear_result(result);
				sprintf(query,"select totpsecret,totpcode from compte where email=\'%s\';",nom);
				db_query(handler,query);
				db_getvalue(result,0,0,totpsecret,40);
				db_getvalue(result,0,1,totpcode,40);
				db_clear_result(result);
				if(strcmp(totpsecret,"")==0) success=1;
      		else
         		{
         		if(strcmp(totpcode,code)==0) success=1;
         		else
            		{
            		strcpy(reponse,"Erreur : Code de vérification erroné ...");
            		strcpy(lienretour,"/reflex/index.html");
            		strcpy(textebouton,"Réessayer");
            		sleep(2);
         			}
					}
				if(success==1)
					{
					if(strcmp(nom,owner)!=0) 
						{
						sprintf(query,"select * from blacklist where blacklister=\'%s\' and blacklisted=\'%s\';",owner,nom);
						db_query(handler,query);
						if(db_ntuples(result)!=0) 
							{
							success=2;
							strcpy(query,"select nom,public from album where compte=\'jhfsouehzviuvber\';");
							}
						db_clear_result(result);
						}
					if(success==1)
						{
						if(strcmp(nom,owner)==0) sprintf(query,"select nom, public from album where compte=\'%s\' order by horodatage desc;",nom);
						else sprintf(query,"select nom, public from album where compte=\'%s\' and public=\'oui\' order by horodatage desc;",owner);
						}
					db_query(handler,query);
					success=1;
					}
				}
			}
		}
	}
strcpy(mdp,mdpsave);
printf("Set-Cookie: reflexemail=%s; HttpOnly; Path=/\n\
Set-Cookie: reflexmdp=%s; HttpOnly; Path=/\n\
Set-Cookie: reflexcode=%s; HttpOnly; Path=/\n\
Set-Cookie: reflexowner=%s; HttpOnly; Path=/\n\
Content-Type: text/html\n\n\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
<title>reflex</title>\n\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n\
<meta charset=\"utf-8\">\n\
<link rel=\"icon\" type=\"image/x-icon\" href=\"/reflex/reflex.png\">\n\
<link rel=\"stylesheet\" href=\"/reflex/themes/reflex.min.css\" />\n\
<link rel=\"stylesheet\" href=\"/reflex/themes/jquery.mobile.icons.min.css\" />\n\
<link rel=\"stylesheet\" href=\"/jq/jquery.mobile.structure-1.4.5.min.css\" />\n\
<script src=\"/jq/demos/js/jquery.min.js\"></script>\n\
<script src=\"/jq/jquery.mobile-1.4.5.min.js\"></script>\n\
</head>\n\
<body>\n\
   <div data-role=\"page\" data-theme=\"b\">\n\
      <div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\">",nom,mdp,code,owner);
if(success==1)
	{
	printf("<h4>Reflex - Partage de photos</h4>\n\
		</div>\n\
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
      <p style=\"text-align:center\"><strong>%s</strong></p>\n",owner);
	for(i=0;i<db_ntuples(result);i++)
		{
		db_getvalue(result,i,0,reponse,20000);
		db_getvalue(result,i,1,reponse2,20000);
		if(strcmp(reponse2,"oui")==0) strcpy(reponse2,"Public");
		else strcpy(reponse2,"Privé");
		printf("<form action=\"/cgi-bin/reflex_album.cgi\" method=\"post\" id=\"form-album%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"b\" data-icon=\"camera\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">%s (%s)</button>\n\
                  </form>\n",i,nom,mdp,code,owner,reponse,reponse,reponse2);
		}
	db_clear_result(result);
   printf("</div>\n\
      <div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
         <div data-role=\"navbar\">\n\
            <ul>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-murpublic\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"n\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"n\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"eye\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Public</button>\n\
                  </form>\n\
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-muramis\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"n\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"y\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"eye\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Amis</button>\n\
                  </form>\n\
               </li>\n",nom,mdp,code,nom,mdp,code);
   if(strcmp(nom,owner)==0)
   	{
   	printf("<li>\n\
                  <form action=\"/cgi-bin/reflex_addalbum.cgi\" method=\"post\" id=\"form-addalbum\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"plus\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Album</button>\n\
                  </form>\n\
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_editalbum.cgi\" method=\"post\" id=\"form-editalbum\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"edit\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Album</button>\n\
                  </form>\n\
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_deletealbum.cgi\" method=\"post\" id=\"form-deletealbum\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"delete\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Album</button>\n\
                  </form>\n\
               </li>\n",nom,mdp,code,nom,mdp,code,nom,mdp,code);
   	}
   printf("</ul>\n\
      	</div> <! navbar>\n\
      </div> <! footer>\n\
   </div> <! page>\n\
</body>\n\
</html>\n");
	}
else
	{
	printf("</div>\n\
      <div role=\"main\" class=\"ui-content\">\n\
      %s<br>\n\
<a href=\"%s\" data-role=\"button\" data-icon=\"back\" data-theme=\"c\">%s</a><br><br>\n",reponse,lienretour,textebouton);
	printf("</div>\n\
<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
	}
}