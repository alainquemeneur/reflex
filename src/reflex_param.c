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
int i,start,success,id,totpactif;
char *query,*res,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *mdp,*nom,*code,*mdpsav,*ami,*digestavatar,*urlavatar;

envoi=read_POST();
start=strlen(envoi);
for(i=0;i<start;i++)
	{
	if(envoi[i]<32) envoi[i]=0;
	}
query_size=atoi(getenv("CONTENT_LENGTH"));
mdp=(char*)malloc(1000+query_size);
mdpsav=(char*)malloc(1000+query_size);
nom=(char*)malloc(1000+query_size);
code=(char*)malloc(1000+query_size);
ami=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
digestavatar=(char*)malloc(20000);
urlavatar=(char*)malloc(20000);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
if(veille_au_grain3(envoi,3,nom,mdp,code,0)==0)
	{
	get_chaine(envoi,4,nom);
	tamb(nom);
	get_chaine(envoi,3,mdp);
	tamb(mdp);
	get_chaine(envoi,2,code);
	tamb(code);
	get_chaine(envoi,1,ami);
	tamb(ami);
	}
strcpy(mdpsav,mdp);
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
				sprintf(query,"select totpsecret,totpcode,digestavatar from compte where email=\'%s\';",nom);
      		db_query(handler,query);
      		db_getvalue(result,0,0,totpsecret,40);
      		db_getvalue(result,0,1,totpcode,40);
      		db_getvalue(result,0,2,digestavatar,20000);
      		sprintf(urlavatar,"/reflex/avatar/%s",digestavatar);
      		db_clear_result(result);
      		if(strcmp(totpsecret,"")==0) 
      			{
      			success=1;
      			totpactif=0;
      			}
      		else
         		{
         		totpactif=1;
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
      			strcpy(mdp,mdpsav);
					printf("Content-Type: text/html\n\n\
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
      <div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\">\n\
		<div data-role=\"navbar\">\n\
            <ul>\n\
               <li>\n\
					<form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-murpublic\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"n\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"back\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Retour au mur</button>\n\
                  </form>\n\
               </li>\n\
            </ul>\n\
         </div>\n\
         </div>\n\
   <div role=\"main\" class=\"ui-content\">\n",nom,mdp,code,ami);
   				if(totpactif==0) printf("<form action=\"/cgi-bin/reflex_active2fa.cgi\" method=\"post\" id=\"form-totp\" data-transition=\"none\" data-rel=\"dialog\">\n\
		<input type=\"hidden\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
		<button type=\"submit\" data-theme=\"c\" data-mini=\"true\" data-inline=\"true\" data-icon=\"lock\">Activer l'authentification 2FA</button>\n\
		</form>\n",nom,mdp,code,ami);
   				else printf("<form action=\"/cgi-bin/reflex_desactive2fa.cgi\" method=\"post\" id=\"form-totp\" data-transition=\"none\" data-rel=\"dialog\">\n\
		<input type=\"hidden\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
        <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
		<button type=\"submit\" data-theme=\"c\" data-mini=\"true\" data-inline=\"true\" data-icon=\"lock\">Désactiver l'authentification 2FA</button>\n\
		</form>\n",nom,mdp,code,ami);
         		printf("<hr><br><p style=\"text-align:center\"><strong>Mon compte</strong></p>\n\
         			<p style=\"text-align:center\"><img src=\"%s\"></p>\n\
         			<form action=\"/cgi-bin/reflex_uploadavatar.cgi\" target=\"upload\" method=\"post\" id=\"form-upload\" data-transition=\"none\" data-rel=\"dialog\">\n\
						<input type=\"hidden\" name=\"txt-nom\" id=\"txt-nom\" value=\"%s\">\n\
        				<input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
        				<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
        				<input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
						<button type=\"submit\" data-theme=\"d\" data-mini=\"false\" data-inline=\"false\" data-icon=\"user\">Changer l'avatar</button>\n\
					</form>\n\
					<hr><br>\n\
               <form autocomplete=\"off\" action=\"/cgi-bin/reflex_updatepassword.cgi\" method=\"post\" id=\"form-profile\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                 	<input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                 	<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                 	<input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
                  <div data-role=\"fieldcontain\">\n\
                  <label for=\"txt-email\">Adresse email *</label>\n\
                  <input type=\"text\" name=\"txt-email\" id=\"txt-email\" maxlength=\"99\" value=\"%s\" disabled>\n\
                  </div>\n\
                  <div data-role=\"fieldcontain\">\n\
                  <label for=\"txt-password\">Mot de passe actuel *</label>\n\
                  <input type=\"password\" name=\"txt-password\" maxlength=\"99\" id=\"txt-password\" value=\"\" required=\"required\">\n\
                  </div>\n\
                  <div data-role=\"fieldcontain\">\n\
                  <label for=\"txt-new\">Nouveau mot de passe</label>\n\
                  <input type=\"password\" name=\"txt-new\" maxlength=\"99\" id=\"txt-new\" value=\"\">\n\
                  </div>\n\
                  <div data-role=\"fieldcontain\">\n\
                  <label for=\"txt-confirm\">Confirmation du mot de passe</label>\n\
                  <input type=\"password\" name=\"txt-confirm\" maxlength=\"99\" id=\"txt-confirm\" value=\"\">\n\
                  </div>\n\
                  <input type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"c\" data-icon=\"action\" value=\"Mettre à jour mon mot de passe\">\n\
                  </form>\n\
      </div>\n",urlavatar,nom,mdp,code,ami,nom,mdp,code,ami,nom);
		printf("<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
      	<h4>Reflex - Partage de photos</h4>\n\
      </div>\n\
   </div></body>\n\
</html>\n");
					}
				}
			}
		db_close(handler);
		}
	}
if(success!=1)
	{	
	printf("Content-Type: text/html\n\n\
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
      <div role=\"main\" class=\"ui-content\">\n\
      %s\n",reponse);
	printf("</div>\n\
<div data-role=\"footer\" class=\"ui-bar\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
	}
}