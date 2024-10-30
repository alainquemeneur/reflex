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
int i,j,start,success,nb_photos,nb_blacklist,nb_followed,aretenir;
char *query,*reponse,*lienretour,*textebouton,*totpsecret,*totpcode;
char *nom,*mdp,*code,*enter,*ami,*mdpsave,*compte,*album,*digest,*txtjaime,*couleur,*txtsuivre,*couleursuivi,*digestavatar,*urlavatar;
struct photo *photos;
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
mdpsave=(char*)malloc(1000+query_size);
code=(char*)malloc(1000+query_size);
enter=(char*)malloc(1000+query_size);
ami=(char*)malloc(1000+query_size);
reponse=(char*)malloc(20000);
digestavatar=(char*)malloc(20000);
urlavatar=(char*)malloc(20000);
lienretour=(char*)malloc(100);
album=(char*)malloc(200);
compte=(char*)malloc(200);
digest=(char*)malloc(100);
textebouton=(char*)malloc(20);
totpsecret=(char*)malloc(40);
totpcode=(char*)malloc(40);
txtjaime=(char*)malloc(40);
couleur=(char*)malloc(40);
couleursuivi=(char*)malloc(40);
txtsuivre=(char*)malloc(40);
query=(char*)malloc(20000+query_size);
if(veille_au_grain3(envoi,5,nom,mdp,code,0)==0)
	{
	get_chaine(envoi,5,nom);
	tamb(nom);
	get_chaine(envoi,4,mdp);
	tamb(mdp);
	get_chaine(envoi,3,code);
	tamb(code);
	get_chaine(envoi,2,enter);
	tamb(enter);
	get_chaine(envoi,1,ami);
	tamb(ami);
	}
else 
	{
	strcpy(enter,"n");
	strcpy(ami,"n");
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
					if(strcmp(enter,"y")==0)
						{
						if(checkcode(totpsecret,code)==0)
							{
							sprintf(query,"update compte set totpcode=\'%s\' where email=\'%s\';",code,nom);
							db_query(handler,query);
							db_clear_result(result);
							}
						else 
							{
							strcpy(reponse,"Erreur : Code de vérification erroné ...");
							strcpy(lienretour,"/reflex/index.html");
							strcpy(textebouton,"Réessayer");
							success=0;
							sleep(2);
							}
						}
					else
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
					}
				if(success==1)
					{
					sprintf(query,"rm /var/www/html/reflex/qr/qr%s.png",nom);
					system(query);
					sprintf(query,"rm /tmp/reflex_totp%s",nom);
					system(query);
					sprintf(query,"select followed from suivi where follower=\'%s\';",nom);
					db_query(handler,query);
					nb_followed=db_ntuples(result);
					followeds=(struct followed *)malloc(nb_followed*sizeof(struct followed));
					for(i=0;i<nb_followed;i++)	
						{
						db_getvalue(result,i,0,reponse,20000);
						strcpy(followeds[i].followed,reponse);
						}
					db_clear_result(result);
					sprintf(query,"select blacklister from blacklist where blacklisted=\'%s\';",nom);
					db_query(handler,query);
					nb_blacklist=db_ntuples(result);
					blacklists=(struct blacklist *)malloc(nb_blacklist*sizeof(struct blacklist));
					for(i=0;i<nb_blacklist;i++) db_getvalue(result,i,0,blacklists[i].blacklister,100);
					db_clear_result(result);
					sprintf(query,"select distinct compte,album,digest,horodatage from photo where (compte=\'%s\' or public=\'oui\') order by horodatage desc;",nom);
					db_query(handler,query);
					nb_photos=db_ntuples(result);
					if(nb_photos>NB_PHOTOS_MAX) nb_photos=NB_PHOTOS_MAX;
					photos=(struct photo *)malloc(nb_photos*sizeof(struct photo));
					start=0;
					for(i=0;i<nb_photos;i++)
						{
						if(strcmp(ami,"n")==0)
							{
							db_getvalue(result,i,0,photos[i].compte,100);
							db_getvalue(result,i,1,photos[i].album,100);
							db_getvalue(result,i,2,photos[i].digest,100);
							photos[i].blacklisted=0;
							for(j=0;j<nb_blacklist;j++)
								{
								if(strcmp(photos[i].compte,blacklists[j].blacklister)==0) photos[i].blacklisted=1;
								}
							}
						else
							{
							db_getvalue(result,i,0,photos[start].compte,100);
							aretenir=0;
							for(j=0;j<nb_followed;j++)
								{
								if(strcmp(photos[start].compte,followeds[j].followed)==0) aretenir=1;
								}
							if(aretenir==1)
								{
								db_getvalue(result,i,1,photos[start].album,100);
								db_getvalue(result,i,2,photos[start].digest,100);
								photos[start].blacklisted=0;
								for(j=0;j<nb_blacklist;j++)
									{
									if(strcmp(photos[start].compte,blacklists[j].blacklister)==0) photos[start].blacklisted=1;
									}
								start++;
								}
							}
						}
					if(strcmp(ami,"y")==0) nb_photos=start;
					db_clear_result(result);
					for(i=0;i<nb_photos;i++)
						{
						if(photos[i].blacklisted!=1)
							{
							sprintf(query,"select compte from aime where owner=\'%s\' and album=\'%s\' and photodigest=\'%s\';",photos[i].compte,photos[i].album,photos[i].digest);
							db_query(handler,query);
							photos[i].jaimes=db_ntuples(result);
							photos[i].jaime=0;
							for(j=0;j<db_ntuples(result);j++)
								{
								db_getvalue(result,j,0,reponse,20000);
								if(strcmp(reponse,nom)==0) photos[i].jaime=1;
								}
							db_clear_result(result);
							}
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
Set-Cookie: reflexowner=%s; HttpOnly; Path=/\n\
Content-Type: text/html\n\n\
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
      <div data-role=\"header\" data-position=\"fixed\" data-theme=\"b\">",nom,mdp,code,nom);
if(success==1)
	{
	printf("<h4>Reflex - Partage de photos</h4>\n\
		</div>\n\
      <div role=\"main\" class=\"ui-content\" data-theme=\"b\">\n");
	for(i=0;i<nb_photos;i++)
		{
		if(photos[i].blacklisted!=1)
			{
			if(photos[i].jaime==0)
				{
				strcpy(txtjaime,"J'aime");
				strcpy(couleur,"c");
				}
			else
				{
				strcpy(txtjaime,"Je n'aime plus");
				strcpy(couleur,"d");
				}
			strcpy(txtsuivre,"Suivre");
			strcpy(couleursuivi,"c");
			for(j=0;j<nb_followed;j++)
				{
				if(strcmp(photos[i].compte,followeds[j].followed)==0) 
					{
					strcpy(txtsuivre,"Ne plus suivre");
					strcpy(couleursuivi,"d");
					}
				}
			sprintf(query,"select digestavatar from compte where email=\'%s\';",photos[i].compte);
			db_query(handler,query);
			db_getvalue(result,0,0,digestavatar,20000);
			db_clear_result(result);
			sprintf(urlavatar,"/reflex/avatar/%s",digestavatar);
			sprintf(reponse,"%s/%s/%s",STORAGE_WEB,photos[i].compte,photos[i].digest);
			printf("<div>\n\
			<img src=\"%s\">\n\
			<form action=\"/cgi-bin/reflex_albums.cgi\" method=\"post\" id=\"form-albums%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
         <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
         <button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"d\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">%s</button>\n\
         </form>\n\
         <form method=\"post\" action=\"/cgi-bin/reflex_view.cgi\" target=\"view\">\n\
			<input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
      	<input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
      	<input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
      	<input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
      	<input type=\"hidden\" name=\"txt-digest\" id=\"txt-digest\" value=\"%s\">\n\
      	<input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
			<input type=\"image\" name=\"%s\" value=\"%s\" src=\"%s.jpg\" style=\"max-width:100%%; display: block;\" loading=\"lazy\">\n\
			</form>\n\
			<br>\n\
			<img src=\"/reflex/heart.png\" style=\"vertical-align:middle;\"> <strong>%d</strong>\n\
			<form action=\"/cgi-bin/reflex_jaime.cgi\" method=\"post\" id=\"form-jaime%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
         <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-album\" id=\"txt-album\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-digest\" id=\"txt-digest\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-destination\" id=\"txt-destination\" value=\"mur\">\n\
			<input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"n\">\n\
			<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"%s\" class=\"ui-shadow\">%s</button>\n\
			</form>\n",urlavatar,i,nom,mdp,code,photos[i].compte,photos[i].compte,nom,mdp,code,photos[i].album,photos[i].digest,photos[i].compte,photos[i].digest,photos[i].digest,reponse,photos[i].jaimes,i,nom,mdp,code,photos[i].compte,photos[i].album,photos[i].digest,couleur,txtjaime);
			if(strcmp(nom,photos[i].compte)!=0) printf("<form action=\"/cgi-bin/reflex_suivre.cgi\" method=\"post\" id=\"form-suivre%d\" data-transition=\"none\" data-rel=\"dialog\">\n\
         <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-owner\" id=\"txt-owner\" value=\"%s\">\n\
         <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
			<button type=\"submit\" data-mini=\"true\" data-inline=\"true\" data-theme=\"%s\" class=\"ui-shadow\">%s</button>\n\
			</form>\n",i,nom,mdp,code,photos[i].compte,ami,couleursuivi,txtsuivre);
			printf("</div>\n<hr>\n");			
			}
		}
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
                  <form action=\"/cgi-bin/reflex_main.cgi\" method=\"post\" id=\"form-muramis\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-enter\" id=\"txt-enter\" value=\"n\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"y\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"eye\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Amis</button>\n\
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
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_social.cgi\" method=\"post\" id=\"form-social\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"user\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Social</button>\n\
                  </form>\n\
               </li>\n\
               <li>\n\
                  <form action=\"/cgi-bin/reflex_param.cgi\" method=\"post\" id=\"form-param\" data-transition=\"none\" data-rel=\"dialog\">\n\
                  <input type=\"hidden\" name=\"txt-compte\" id=\"txt-compte\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-password\" id=\"txt-password\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-code\" id=\"txt-code\" value=\"%s\">\n\
                  <input type=\"hidden\" name=\"txt-ami\" id=\"txt-ami\" value=\"%s\">\n\
                  <button type=\"submit\" data-mini=\"false\" data-inline=\"false\" data-theme=\"d\" data-icon=\"gear\" class=\"ui-shadow\" style=\"max-width:100%%; border: 0; object-fit: contain;\">Compte</button>\n\
                  </form>\n\
               </li>\n\
            </ul>\n\
      	</div> <! navbar>\n\
      </div> <! footer>\n\
   </div> <! page>\n\
</body>\n\
</html>\n",nom,mdp,code,nom,mdp,code,nom,mdp,code,nom,nom,mdp,code,ami,nom,mdp,code,ami);
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