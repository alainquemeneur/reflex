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
int i,start;
char *query,*res,*reponse,*lienretour,*textebouton;
char *nom,*mdp1,*mdp2,*digestavatar,*mdpsav;

envoi=read_POST();
start=strlen(envoi);
for(i=0;i<start;i++)
	{
	if(envoi[i]<32) envoi[i]=0;
	}
query_size=atoi(getenv("CONTENT_LENGTH"));
nom=(char*)malloc(1000+query_size);
mdp1=(char*)malloc(1000+query_size);
mdp2=(char*)malloc(1000+query_size);
mdpsav=(char*)malloc(1000+query_size);
digestavatar=(char*)malloc(20000);
res=(char*)malloc(100);
reponse=(char*)malloc(20000);
lienretour=(char*)malloc(100);
textebouton=(char*)malloc(20);
query=(char*)malloc(20000+query_size);
get_chaine(envoi,3,nom);
tamb(nom);
if(nom[strlen(nom)-1]==' ') nom[strlen(nom)-1]=0;
get_chaine(envoi,2,mdp2);
tamb(mdp2);
get_chaine(envoi,1,mdp1);
tamb(mdp1);
strcpy(mdpsav,mdp1);
strcpy(digestavatar,"man.png");
if(strcmp(mdp1,mdp2)!=0) 
		{
		strcpy(reponse,"Erreur : Création de compte impossible, les deux mots de passe ne correspondent pas ...");
		strcpy(lienretour,"/reflex/reflex_newprofile.html");
		strcpy(textebouton,"Réessayer");
		}
else
		{
		if((handler=db_opendatabase("reflex","localhost","reflex",PASSWORD))==NULL)
			{
			strcpy(reponse,"Erreur : Impossible d'accéder à la base de données");
			strcpy(lienretour,"/reflex/reflex_newprofile.html");
			strcpy(textebouton,"Réessayer");
			}
		else
			{
			sprintf(query,"select * from compte where email=\'%s\';",nom);
			db_query(handler,query);
			if(db_ntuples(result)!=0)
				{
				strcpy(reponse,"Erreur : Cette adresse email est déjà enregistrée ...");
				strcpy(lienretour,"/reflex/reflex_newprofile.html");
				strcpy(textebouton,"Réessayer");
				db_clear_result(result);
				db_close(handler);
				}
			else
				{
				db_clear_result(result);
				hache(mdp1);
				sprintf(query,"insert into compte values (\'%s\',\'%s\','oui',\'\',\'\',\'%s\');",nom,mdp1,digestavatar);
				db_query(handler, query);
				db_clear_result(result);
				sprintf(query,"insert into suivi values (\'%s\',\'%s\');",nom,nom);
				db_query(handler, query);
				db_clear_result(result);
				strcpy(reponse,"Félicitations ! Votre compte a bien été créé");
				strcpy(lienretour,"/reflex/index.html");
				strcpy(textebouton,"Se connecter");
				db_close(handler);
				}
			}
		}
printf("Content-Type: text/html\n\n\
<!doctype html>\n\
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
<p>%s</p><br>\n\
<a href=\"%s\" data-role=\"button\" data-icon=\"back\" data-theme=\"c\">%s</a><br><br>\n\
</div>\n\
<div data-role=\"footer\" class=\"ui-bar\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>",reponse,lienretour,textebouton);
}