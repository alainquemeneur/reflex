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
int i,start,success,nb_likers;
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*owner,*album,*mdpsave,*estpublic,*digest,*txtjaime,*couleur;
struct photo Photo;
struct liker *likers;

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
lienretour=(char*)malloc(100);
digest=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
estpublic=(char*)malloc(40);
txtjaime=(char*)malloc(40);
couleur=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
get_chaine(envoi,8,nom);
tamb(nom);
get_chaine(envoi,7,mdp);
tamb(mdp);
get_chaine(envoi,6,code);
tamb(code);
get_chaine(envoi,5,album);
tamb(album);
get_chaine(envoi,4,digest);
tamb(digest);
get_chaine(envoi,3,owner);
tamb(owner);
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
            			success=0;
         				}
					}
				if(success==1)
					{
					sprintf(query,"select compte from aime where owner=\'%s\' and album=\'%s\' and photodigest=\'%s\';",owner,album,digest);
					db_query(handler,query);
					Photo.jaimes=db_ntuples(result);
					Photo.jaime=0;
					likers=(struct liker *)malloc(Photo.jaimes*sizeof(struct liker));
					nb_likers=db_ntuples(result);
					for(i=0;i<db_ntuples(result);i++)
						{
						db_getvalue(result,i,0,reponse,20000);
						strcpy(likers[i].compte,reponse);
						if(strcmp(reponse,nom)==0) Photo.jaime=1;
						}
					db_clear_result(result);
					sprintf(query,"select public from album where compte=\'%s\' and nom=\'%s\';",owner,album);
					db_query(handler,query);
					if(db_ntuples(result)==0) success=0;
					db_clear_result(result);
					if(success==1)
						{
						sprintf(reponse,"%s/%s/%s",STORAGE_WEB,owner,digest);
						strcpy(mdp,mdpsave);
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
<img src=\"%s\" style=\"max-width:100%%;\"><br>\n",reponse);
if(Photo.jaime==0)
	{
	strcpy(txtjaime,"J'aime");
	strcpy(couleur,"c");
	}
else
	{
	strcpy(txtjaime,"Je n'aime plus");
	strcpy(couleur,"d");
	}
printf("<img src=\"/reflex/heart.png\" style=\"vertical-align:middle;\"> <strong>%d</strong>\n\
<form action=\"/cgi-bin/reflex_jaime.cgi\" method=\"post\" id=\"form-jaime\" data-transition=\"none\" data-rel=\"dialog\">\n\
<input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-digest\" id=\"txt-digest\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-destination\" id=\"txt-destination\" value=\"view\">\n\
<input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"n\">\n\
<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"%s\" class=\"ui-shadow\">%s</button>\n\
</form>\n",Photo.jaimes,nom,mdp,code,owner,album,digest,couleur,txtjaime);
printf("Ils aiment cette photo :<br>\n");
for(i=0;i<nb_likers;i++) printf("<button type=\"button\" data-mini=\"true\" data-inline=\"true\" data-theme=\"b\" class=\"ui-shadow\" disabled>%s</button>\n",likers[i].compte);
if(strcmp(nom,owner)==0) printf("<form action=\"/cgi-bin/reflex_deletephoto.cgi\" method=\"post\" id=\"form-deletephoto\" data-transition=\"none\" data-rel=\"dialog\">\n\
<input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
<input type=\"hidden\" name=\"txt-digest\" id=\"txt-digest\" value=\"%s\">\n\
<button type=\"submit\" data-mini=\"false\" data-inline=\"true\" data-theme=\"c\" data-icon=\"delete\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Supprimer cette photo</button>\n\
</form>\n",nom,mdp,code,album,digest);
printf("</body>\n\
</html>\n");
						}
					}
				}
			}
		}
	}
}