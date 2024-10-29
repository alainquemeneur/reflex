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
#include "crypto.c"

int main()
{
char *envoi;
int i,start,success;
char *query,*reponse,*lienretour,*textebouton,*totpsecret;
char *nom,*mdp,*mdpsav;
char *code;

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
mdpsav=(char*)malloc(1000+query_size);
code=(char*)malloc(1000+query_size);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
reponse=(char*)malloc(200);
query=(char*)malloc(20000+query_size);
get_chaine(envoi,2,nom);
tamb(nom);
if(nom[strlen(nom)-1]==' ') nom[strlen(nom)-1]=0;
get_chaine(envoi,1,mdp);
tamb(mdp);
strcpy(mdpsav,mdp);
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
				sprintf(query,"select totpsecret from compte where email=\'%s\';",nom);
				db_query(handler,query);
				db_getvalue(result,0,0,totpsecret,40);
				db_clear_result(result);
				db_close(handler);
				if(strcmp(totpsecret,"")==0) 
					{
					success=1;
					}
				else success=2;
				}
			}
		}
	}
get_chaine(envoi,1,mdp);
tamb(mdp);
printf("Content-Type: text/html\n\n\
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
      <div role=\"main\" class=\"ui-content\">\n");
if(success==0) printf("%s\n\
      <a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Ressayez</a>",reponse);
if(success==1) printf("Votre compte n'est pas protégé par un deuxième facteur d'authentification, ce qui améliorerait pourtant sa sécurité. Vous pouvez activer cette fonctionnalité depuis la page de votre profil<br>\n\
						<form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-connect\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"\">\n\
                  <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"n\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"n\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"c\" data-icon=\"action\" class=\"ui-shadow\">Accéder à mon compte</button>\n\
                  </form>\n",nom,mdp);
if(success==2) 
	{
	printf("<form autocomplete=\"off\" action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form\" data-transition=\"none\" data-rel=\"dialog\">\n\
            <input type=\"hidden\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\">\n\
            <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
            <div data-role=\"fieldcontain\">\n\
               <label for=\"txt-code\">Code de vérification</label>\n\
               <input type=\"text\" name=\"txt-code\" maxlength=\"6\" id=\"txt-code\" value=\"\">\n\
            </div>\n\
            <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"y\">\n\
            <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"n\">\n\
            <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"c\" data-icon=\"action\">Accéder à mon compte</button>\n\
         </form>\n",nom,mdp);
	sprintf(reponse,"rm /var/www/html/reflex/qr/qr%s.png",nom);
	system(reponse);
	}
printf("</div>\n\
<div data-role=\"footer\" class=\"ui-bar\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
}