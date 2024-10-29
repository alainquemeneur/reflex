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
int i,start,success,fd,nb;
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*mdpsave,*ami;

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
ami=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
strcpy(ami,"n");
if(veille_au_grain3(envoi,4,nom,mdp,code,0)==0)
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
					sprintf(reponse,"openssl rand 10 | base32 > \'/tmp/reflex_totp%s\'",nom);
					system(reponse);
					sprintf(reponse,"/tmp/reflex_totp%s",nom);
					fd=open(reponse,O_RDONLY);
					nb=read(fd,reponse,100);
					close(fd);
					reponse[nb]=0;
					strcpy(lienretour,reponse);
					sprintf(reponse,"echo \'otpauth://totp/Reflex (%s)?secret=%s\' | qrencode -t png -o \'/var/www/html/reflex/qr/qr%s.png\'",nom,lienretour,nom);
					system(reponse);
         		sprintf(query,"update compte set totpsecret=\'%s\', totpcode=\'\' where email=\'%s\';",lienretour,nom);
         		db_query(handler,query);
         		db_clear_result(result);
         		strcpy(code,"");
         		sprintf(reponse,"L'authentification 2FA a été activée pour protéger votre compte. Merci de bien vouloir scanner le QR code ci-dessous avec l'application Google Authenticator sur votre smartphone, ou entrez y manuellement le code secret qui vous a été attribué.\n\
<br><img src=\"/reflex/qr/qr%s.png\"><br>Code secret : %s",nom,lienretour);
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
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n\
      %s\n\
      </div>\n\
      <div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
         <h4>Reflex - Partage de photos</h4>\n\
      </div> <! footer>\n\
   </div> <! page>\n\
</body>\n\
</html>\n",nom,mdp,code,ami,reponse);
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