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
int i,start,success,nb_messages,lastid,idfirst,idlast,id;
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*mdpsav,*album,*nomalbum,*estpublic,*horodatage,*jour,*mois,*annee,*heure,*minute,*seconde;
time_t tim;
struct tm *TM;

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
nomalbum=(char*)malloc(1000+query_size);
album=(char*)malloc(1000+query_size);
estpublic=(char*)malloc(1000+query_size);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
reponse=(char*)malloc(20000);
horodatage=(char*)malloc(200);
jour=(char*)malloc(10);
mois=(char*)malloc(10);
annee=(char*)malloc(20);
heure=(char*)malloc(10);
minute=(char*)malloc(10);
seconde=(char*)malloc(10);
query=(char*)malloc(20000+query_size);
veille_au_grain3(envoi,6,nom,mdp,code,1);
get_chaine(envoi,6,nom);
tamb(nom);
get_chaine(envoi,5,mdp);
tamb(mdp);
get_chaine(envoi,4,code);
tamb(code);
get_chaine(envoi,3,album);
tamb(album);
get_chaine(envoi,2,nomalbum);
tamb(nomalbum);
get_chaine(envoi,1,estpublic);
tamb(estpublic);
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
         		sprintf(query,"update album set nom=\'%s\',public=\'%s\' where compte=\'%s\' and nom=\'%s\';",nomalbum,estpublic,nom,album);
         		db_query(handler,query);
         		db_clear_result(result);
         		sprintf(query,"select * from album where compte=\'%s\' and nom=\'%s\' and public=\'%s\';",nom,nomalbum,estpublic);
         		db_query(handler,query);
         		if(db_ntuples(result)==0)
         			{
         			success=0;
         			strcpy(reponse,"La modification n'a pas été réalisée");
         			db_clear_result(result);
         			}
         		else
         			{
         			db_clear_result(result);
         			sprintf(query,"update photo set album=\'%s\', public=\'%s\' where compte=\'%s\' and album=\'%s\';",nomalbum,estpublic,nom,album);
         			db_query(handler,query);
         			db_clear_result(result);
         			strcpy(reponse,"L'album a été mis à jour");
         			}
					}				
				db_close(handler);
				}
			}
		}
	}
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
      <div role=\"main\" class=\"ui-content\">\n\
      %s\n",nom,mdp,code,nom,reponse);
printf("</div>\n\
<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
}