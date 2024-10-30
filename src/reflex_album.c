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
#include "specific.h"
#include "totp.c"
#include "crypto.c"

int main()
{
char *envoi;
int i,start,success;
char *query,*reponse,*reponse2,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*owner,*album,*mdpsave,*estpublic,*digest;

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
album=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
reponse2=(char*)malloc(20000);
lienretour=(char*)malloc(100);
digest=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
estpublic=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
if(veille_au_grain5(envoi,5,nom,mdp,code,owner,album,0)==0)
	{
	get_chaine(envoi,5,nom);
	tamb(nom);
	get_chaine(envoi,4,mdp);
	tamb(mdp);
	get_chaine(envoi,3,code);
	tamb(code);
	get_chaine(envoi,2,owner);
	tamb(owner);
	get_chaine(envoi,1,album);
	tamb(album);
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
					sprintf(query,"select public from album where compte=\'%s\' and nom=\'%s\';",owner,album);
					db_query(handler,query);
					if(db_ntuples(result)!=0) 
						{
						db_getvalue(result,0,0,estpublic,40);
						if(strcmp(estpublic,"oui")==0) strcpy(estpublic,"Public");
						else strcpy(estpublic,"Privé");
						}
					else
						{
						success=0;
						strcpy(reponse,"Cet album n'existe pas");
						}
					db_clear_result(result);
					if(strcmp(nom,owner)!=0) 
						{
						sprintf(query,"select * from blacklist where blacklister=\'%s\' and blacklisted=\'%s\';",owner,nom);
						db_query(handler,query);
						if(db_ntuples(result)!=0) 
							{
							success=2;
							strcpy(query,"select digest from photo where compte=\'jhfsouehzviuvber\';");
							}
						db_clear_result(result);
						}
					if(success==1)
						{
						if(strcmp(nom,owner)==0) sprintf(query,"select digest from photo where compte=\'%s\' and album=\'%s\' order by horodatage desc;",nom,album);
						else sprintf(query,"select digest from photo where compte=\'%s\' and album=\'%s\' and public=\'oui\' order by horodatage desc;",owner,album);
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
Set-Cookie: reflexalbum=%s; HttpOnly; Path=/\n\
Content-Type: text/html\n\n\
<!DOCTYPE html>\n\
<html>\n\
<head>\n\
<title>Reflex</title>\n\
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
      <div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\">",nom,mdp,code,owner,album);
if(success==1)
	{
	if(strcmp(owner,nom)!=0) printf("<h4>Reflex - Partage de photos</h4>\n\
		</div>\n\
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
      <p style=\"text-align:center\"><strong>%s</strong></p>\n\
      <p style=\"text-align:center\"><strong>%s</strong></p>\n",owner,album);
   else printf("<h4>Reflex - Partage de photos</h4>\n\
		</div>\n\
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
      <p style=\"text-align:center\"><strong>%s</strong></p>\n\
      <p style=\"text-align:center\"><strong>%s (%s)</strong></p>\n",owner,album,estpublic);
	printf("<div style=\"width:100%%; text-align:center;\">\n\
      <table width=100%% CELLSPACING=\"0\" CELLPADDING=\"0\">\n");
	for(i=0;i<db_ntuples(result);i++)
		{
		if(i%3==0) printf("<tr>\n");
		db_getvalue(result,i,0,digest,100);
		sprintf(reponse,"%s/%s/%s",STORAGE_WEB,owner,digest);
		//printf("<td width=30%%><a href=\"/cgi-bin/reflex_view.cgi?image=%s\" target=\"view\"><img src=\"%s\"  style=\"max-width:100%%; display: block;\"></a></td>\n",reponse,reponse);
		printf("<td width=30%%><form id=\"%s\" method=\"post\" action=\"/cgi-bin/reflex_view.cgi\" target=\"view\">\n\
		<input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
      <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
      <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
      <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
      <input type=\"hidden\" name=\"txt-digest\" id=\"txt-digest\" value=\"%s\">\n\
      <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
  		<input type=\"image\" name=\"%s\" value=\"%s\" src=\"%s.jpg\" style=\"max-width:100%%; display: block;\" loading=\"lazy\">\n\
		</form>\n\
		</td>\n",digest,nom,mdp,code,album,digest,owner,digest,digest,reponse);
		if(i%3==2) printf("</tr>\n");
		}
	if((i-1)%3!=2) printf("</tr>\n");
	printf("</table>\n\
	</div>\n");
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
                  <form action=\"/cgi-bin/reflex_album.cgi\" method=\"post\" id=\"form-album\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"refresh\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Rafraichir</button>\n\
                  </form>\n\
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_albums.cgi\" method=\"post\" id=\"form-albums\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"camera\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Albums</button>\n\
                  </form>\n\
               </li>\n",nom,mdp,code,nom,mdp,code,owner,album,nom,mdp,code,owner);
   if(strcmp(nom,owner)==0)
   	{
   	printf("<li>\n\
   					<form target=\"upload\" action=\"/cgi-bin/reflex_doeditalbum.cgi\" method=\"post\" id=\"form-doeditalbum\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"edit\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Album</button>\n\
                  </form>\n\
   				</li>\n\
   				<li>\n\
                  <form target=\"upload\" action=\"/cgi-bin/reflex_addphoto.cgi\" method=\"post\" id=\"form-addphoto\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"plus\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Photo</button>\n\
                  </form>\n\
               </li>\n",nom,mdp,code,album,nom,mdp,code,album);
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