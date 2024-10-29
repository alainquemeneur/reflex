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
#include "crypto.c"

int main()
{
char *envoi;
int i,j,start,success,nb_followed,nb_blacklist;
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*mdpsav,*ami,*email,*txtsuivre,*couleur;
struct blacklist *blacklists;
struct followed *followeds;

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
ami=(char*)malloc(1000+query_size);
lienretour=(char*)malloc(100);
email=(char*)malloc(100);
txtsuivre=(char*)malloc(100);
couleur=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
reponse=(char*)malloc(20000);
query=(char*)malloc(20000+query_size);
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
else strcpy(ami,"n");
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
        			sprintf(query,"select followed,follower from suivi where follower=\'%s\' or followed=\'%s\';",nom,nom);
					db_query(handler,query);
					nb_followed=db_ntuples(result);
					followeds=(struct followed *)malloc(nb_followed*sizeof(struct followed));
					for(i=0;i<nb_followed;i++)	
						{
						db_getvalue(result,i,0,reponse,20000);
						strcpy(followeds[i].followed,reponse);
						db_getvalue(result,i,1,reponse,20000);
						strcpy(followeds[i].follower,reponse);
						}
					db_clear_result(result);
					sprintf(query,"select blacklisted from blacklist where blacklister=\'%s\';",nom);
					db_query(handler,query);
					nb_blacklist=db_ntuples(result);
					blacklists=(struct blacklist *)malloc(nb_blacklist*sizeof(struct blacklist));
					for(i=0;i<nb_blacklist;i++) db_getvalue(result,i,0,blacklists[i].blacklister,100);
					db_clear_result(result);
					sprintf(query,"select email from compte where email!=\'%s\' order by email asc;",nom);
					db_query(handler,query);
					strcpy(reponse,"");
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
                  <form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-mur\" data-transition=\"none\" data-rel=\"dialog\">\n\
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
      %s\n",nom,mdp,code,ami,reponse);
for(i=0;i<db_ntuples(result);i++)
	{
	db_getvalue(result,i,0,email,100);
	printf("%s \n",email);
	start=0;
	for(j=0;j<nb_followed;j++)
		{
		if(strcmp(followeds[j].followed,email)==0) start=1;
		}
	if(start==0) 
		{
		strcpy(txtsuivre,"Suivre");
		strcpy(couleur,"c");
		}
	else
		{
		strcpy(txtsuivre,"Ne plus suivre");
		strcpy(couleur,"d");
		}
	printf("<form action=\"/cgi-bin/reflex_suivre.cgi\" method=\"post\" id=\"form-suivre%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
         <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
			<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"%s\" class=\"ui-shadow\">%s</button>\n\
			</form>\n",i,nom,mdp,code,email,ami,couleur,txtsuivre);
	start=0;
	for(j=0;j<nb_blacklist;j++)
		{
		if(strcmp(blacklists[j].blacklister,email)==0) start=1;
		}
	if(start==0) 
		{
		strcpy(txtsuivre,"Bloquer");
		strcpy(couleur,"c");
		}
	else
		{
		strcpy(txtsuivre,"Ne plus bloquer");
		strcpy(couleur,"d");
		}
	printf("<form action=\"/cgi-bin/reflex_blacklister.cgi\" method=\"post\" id=\"form-bloquer%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
         <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
			<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"%s\" class=\"ui-shadow\">%s</button>\n\
			</form>\n",i,nom,mdp,code,email,ami,couleur,txtsuivre);
	start=0;
	for(j=0;j<nb_followed;j++)
		{
		if(strcmp(followeds[j].follower,email)==0) start=1;
		}
	if(start!=0) printf("<button data-mini=\"true\" data-inline=\"true\" data-theme=\"d\" class=\"ui-shadow\">Vous suit</button>\n");
	printf("<hr>\n");
	}
printf("</div>\n\
<div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
<h4>Reflex - Partage de photos</h4>\n\
</div>\n\
</div>\n\
</body>\n\
</html>\n");
}