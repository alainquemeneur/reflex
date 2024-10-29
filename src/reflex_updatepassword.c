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
char *query,*reponse,*totpsecret,*totpcode,*lienretour,*textebouton;
char *nom,*mdp,*code,*ami,*old,*mdp1,*mdp2;

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
code=(char*)malloc(1000+query_size);
ami=(char*)malloc(1000+query_size);
old=(char*)malloc(1000+query_size);
mdp1=(char*)malloc(1000+query_size);
mdp2=(char*)malloc(1000+query_size);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
lienretour=(char*)malloc(300);
textebouton=(char*)malloc(100);
reponse=(char*)malloc(20000);
query=(char*)malloc(20000+query_size);
veille_au_grain3(envoi,7,nom,mdp,code,1);
get_chaine(envoi,7,nom);
tamb(nom);
get_chaine(envoi,6,mdp);
tamb(mdp);
get_chaine(envoi,5,code);
tamb(code);
get_chaine(envoi,4,ami);
tamb(ami);
get_chaine(envoi,3,old);
tamb(old);
get_chaine(envoi,2,mdp1);
tamb(mdp1);
get_chaine(envoi,1,mdp2);
tamb(mdp2);
hache(mdp);
hache(mdp1);
hache(mdp2);
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
            		sleep(2);
            		}
         		}
         	if(success==1)
         		{
					if(strcmp(old,"")!=0)
						{
						hache(old);
						sprintf(query,"select * from compte where email=\'%s\' and mdp=\'%s\';",nom,old);
						db_query(handler,query);
						if(db_ntuples(result)!=1)
							{
							success=2;
							strcpy(reponse,"Erreur : Mauvais mot de passe ...");
							}
						else success=3;
						db_clear_result(result);
						}
					else 
						{
						strcpy(reponse,"Votre profil n'a pas été mis à jour ...");
						success=4;
						}
					if(success==3)
						{
						if(strcmp(mdp1,mdp2)!=0) strcpy(reponse,"Erreur : Les deux mots de passe ne correspondent pas ...");
						else
							{
							success=1;
							get_chaine(envoi,1,mdp2);
							tamb(mdp2);
							if(strcmp(mdp2,"")!=0) 
								{
								sprintf(query,"update compte set mdp=\'%s\' where email=\'%s\';",mdp1,nom);
								db_query(handler,query);
								db_clear_result(result);
								success=5;
								}
							strcpy(reponse,"Votre mot de passe a bien été mis à jour ...");
							}
						}
					}
				}
			}
		}
	}
if(success==5) 
	{
	get_chaine(envoi,2,mdp);
	tamb(mdp);
	}
else 
	{
	get_chaine(envoi,6,mdp);
	tamb(mdp);
	}
printf("Set-Cookie: reflexmdp=%s; HttpOnly; Path=/\n\
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
      <div role=\"main\" class=\"ui-content\">\n\
      %s\n",mdp,nom,mdp,code,ami,reponse);
printf("</div>\n\
<div data-role=\"footer\" class=\"ui-bar\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
}