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
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*album,*mdpsave,*estpublic;

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
album=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
estpublic=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
get_chaine(envoi,4,nom);
tamb(nom);
get_chaine(envoi,3,mdp);
tamb(mdp);
get_chaine(envoi,2,code);
tamb(code);
get_chaine(envoi,1,album);
tamb(album);
strcpy(mdpsave,mdp);
hache(mdp);
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
				success=1;
				if(strcmp(totpsecret,"")!=0)
					{
					if(strcmp(totpcode,code)!=0)
						{
						strcpy(reponse,"Erreur : Code de vérification erroné ...");
						strcpy(lienretour,"/reflex/index.html");
						strcpy(textebouton,"Réessayer");
						success=0;
						sleep(2);
						}
					}
				if(success==1)
					{
					sprintf(query,"select public from album where compte=\'%s\' and nom=\'%s\';",nom,album);
					db_query(handler,query);
					if(db_ntuples(result)!=0) db_getvalue(result,0,0,estpublic,40);
					else
						{
						success=0;
						strcpy(reponse,"Cet album n'existe pas");
						}
					}
				}
			}
		}
	}
strcpy(mdp,mdpsave);
printf("Set-Cookie: reflexemail=%s; HttpOnly; Path=/\n\
Set-Cookie: reflexmdp=%s; HttpOnly; Path=/\n\
Set-Cookie: reflexcode=%s; HttpOnly; Path=/\n\
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
      <div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\">",nom,mdp,code);
if(success==1)
	{
	printf("<div data-role=\"navbar\">\n\
            <ul>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_albums.cgi\" method=\"post\" id=\"form-albums\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"back\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Retour aux albums</button>\n\
                  </form>\n\
               </li>\n\
            </ul>\n\
      </div>\n\
      </div>\n\
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
      <form autocomplete=\"off\" action=\"/cgi-bin/reflex_doeditalbum2.cgi\" method=\"post\" id=\"formdoeditalbum2\" data-transition=\"none\" data-rel=\"dialog\">\n\
      		<input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
            <div data-role=\"fieldcontain\">\n\
               <label for=\"txt-site\">Nom de l'album</label>\n\
               <input type=\"text\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\" maxlength=\"99\" required=\"required\">\n\
            </div>\n\
            <fieldset data-role=\"controlgroup\">\n\
            <legend>Visibilité de l'album</legend>\n",nom,mdp,code,nom,nom,mdp,code,album,album);
	if(strcmp(estpublic,"oui")!=0) printf("<input type=\"radio\" name=\"radio-choice-m\" id=\"radio-choice-m1\" value=\"non\" data-theme=\"b\" checked=\"checked\">\n\
            <label for=\"radio-choice-m1\">Privé</label>\n\
				<input type=\"radio\" name=\"radio-choice-m\" id=\"radio-choice-m2\" value=\"oui\" data-theme=\"b\">\n\
            <label for=\"radio-choice-m2\">Public</label>\n");
	else printf("<input type=\"radio\" name=\"radio-choice-m\" id=\"radio-choice-m1\" value=\"non\" data-theme=\"b\">\n\
            <label for=\"radio-choice-m1\">Privé</label>\n\
				<input type=\"radio\" name=\"radio-choice-m\" id=\"radio-choice-m2\" value=\"oui\" data-theme=\"b\" checked=\"checked\">\n\
            <label for=\"radio-choice-m2\">Public</label>\n");
	printf("</fieldset>\n\
            <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"c\" data-icon=\"action\">Modifier l'album</button>\n\
         </form>\n\
      </div>\n\
      <div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
         <h4>Reflex - Partage de photos</h4>\n\
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