#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int query_size;

void message_service(char *message);

int hache(char *entree)
{
char *buffer;
int fd,nb,i,start;

buffer=(char *)malloc(1100);
sprintf(buffer,"/tmp/reflex_hash_in_%d",getpid());
fd=open(buffer,O_CREAT | O_TRUNC | O_WRONLY, 0644);
write(fd,entree,strlen(entree));
close(fd);
sprintf(buffer,"/usr/bin/openssl dgst -sha256 \'/tmp/reflex_hash_in_%d\' > \'/tmp/reflex_hash_out_%d\'",getpid(),getpid());
system(buffer);
sprintf(buffer,"/tmp/reflex_hash_out_%d",getpid());
fd=open(buffer,O_RDONLY);
nb=read(fd,buffer,1000);
buffer[nb]=0;
close(fd);
i=0;
while(buffer[i]!='=') i++;
i+=2;
start=i;
while(buffer[i]!=0x0A)
	{
	buffer[i-start]=buffer[i];
	i++;
	}
buffer[i-start]=0;
strcpy(entree,buffer);
sprintf(buffer,"/tmp/reflex_hash_in_%d",getpid());
unlink(buffer);
sprintf(buffer,"/tmp/reflex_hash_out_%d",getpid());
unlink(buffer);
free(buffer);
}

void Send(char *ch)
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
%s<br>\n\
</body>\n\
</html>\n",ch);
exit(0);
}

int get_weekday(char * str) {
  struct tm tm;
  memset((void *) &tm, 0, sizeof(tm));
  if (strptime(str, "%Y-%m-%d", &tm) != NULL) {
    time_t t = mktime(&tm);
    if (t >= 0) {
      return localtime(&t)->tm_wday; // Sunday=0, Monday=1, etc.
    }
  }
  return -1;
}

void alerte(char *ch)
{
printf("%s\n",ch);
}

void elimine_blancs(char *chaine)
{
int i,start;
char *res;

res=(char *)malloc(strlen(chaine)+1);
i=start=0;
do
	{
	res[start]=chaine[i];
	if(res[start]!=' ') start++;
	i++;
	}
while(i<=strlen(chaine));
strcpy(chaine,res);
free(res);
}

void prepare_contenu(char *chaine)
{
int i,start;
char *res;

res=(char *)malloc(strlen(chaine)*2);
i=start=0;
do
	{
	if(chaine[i]=='\'')
		{
		res[start]='\\';
		start++;
		}
	res[start]=chaine[i];
	start++;
	i++;
	}
while(i<=strlen(chaine));
strcpy(chaine,res);
free(res);
}

void magiczero(int num, char *chaine)
{
if(num<10) sprintf(chaine,"0%d",num);
else sprintf(chaine,"%d",num);
}

void url_decode(char *chaine)
{
int i,start;
char *res,buf[50];

start=i=0;
res=(char *)malloc(10*strlen(chaine));
while(i<=strlen(chaine))
	{
	if(chaine[i]!='%') 
		{
		res[start]=chaine[i];
		i++;
		start++;
		}
	else
		{
		if(chaine[i+1]!='3' || chaine[i+2]!='C')
			{
			sprintf(buf,"0x%c%c",chaine[i+1],chaine[i+2]);
			res[start]=strtol(buf,NULL,16);
			start++;
			i = i+3;
			}
		else
			{
			res[start]='&';
			res[start+1]='l';
			res[start+2]='t';
			res[start+3]=';';
			start = start+4;
			i = i+3;
			}
		}
	}
strcpy(chaine,res);
free(res);
}

char* read_POST() 
{
query_size=atoi(getenv("CONTENT_LENGTH"));
char* query_string = (char*) malloc(query_size);
if (query_string != NULL) fread(query_string,query_size,1,stdin);
return query_string;
}

int max_getchaine(char *chaine)
{
int i,somme;

somme=0;
for(i=0;i<strlen(chaine);i++)
	{
	if(chaine[i]=='=') somme++;
	}
return(somme);
}

int get_chaine(char *chaine,int pos,char *resultat)
{
int i,n,start;

i=strlen(chaine);
for(n=0; n<pos;n++) 
	{
	while(chaine[i]!='=') i--;
	i--;
	}
i+=2;
start=i;
do
	{
	resultat[i-start]=chaine[i];
	i++;
	}
while(chaine[i-1]!=0 && chaine[i-1]!='&');
resultat[i-start-1]=0;
}

int get_chaine2(char *chaine,int pos,char *resultat)
{
int i,n,start;

i=strlen(chaine);
for(n=0; n<pos;n++) 
	{
	while(chaine[i]!='&' && i>=0) i--;
	i--;
	}
i+=2;
start=i;
do
	{
	resultat[i-start]=chaine[i];
	i++;
	}
while(chaine[i-1]!=0 && chaine[i-1]!='=');
resultat[i-start-1]=0;
}

void klog(char *ph)
{
int fd;
char a;

a='\n';
fd=open("/tmp/res",O_CREAT|O_WRONLY,0644);
write(fd,ph,strlen(ph));
write(fd,&a,1);
close(fd);
}

void tamb(char *avatar)
{
int i;

for(i=0;i<strlen(avatar);i++)
	{
	if(avatar[i]=='+') avatar[i]=' ';
	}
url_decode(avatar);
prepare_contenu(avatar);
}

int get_cookie_value(char *chaine,int pos,char *resultat)
{
int i,n,start;

i=strlen(chaine);
for(n=0; n<pos;n++) 
	{
	while(chaine[i]!='=') i--;
	i--;
	}
i+=2;
start=i;
do
	{
	resultat[i-start]=chaine[i];
	i++;
	}
while(chaine[i-1]!=0 && chaine[i-1]!=';');
resultat[i-start-1]=0;
}

int get_cookie_name(char *chaine,int pos,char *resultat)
{
int i,n,start;

i=strlen(chaine);
for(n=0; n<pos;n++) 
	{
	while(chaine[i]!=';' && i>=0) i--;
	i--;
	}
if(i==-2) i=0;
else i+=3;
start=i;
do
	{
	resultat[i-start]=chaine[i];
	i++;
	}
while(chaine[i-1]!=0 && chaine[i-1]!='=');
resultat[i-start-1]=0;
}

int get_nb_cookies(char *chaine)
{
int i,somme;

somme=0;
for(i=0;i<strlen(chaine);i++)
	{
	if(chaine[i]=='=') somme++;
	}
return(somme);
}

int veille_au_grain3(char *envoi, int nb, char *nom, char *mdp, char *code, int type)
{
int i,somme,nb_cookies,cookie_id;
char *cookie_name;

i=somme=0;
while(i<strlen(envoi))
	{
	if(envoi[i]=='=') somme++;
	if(somme==nb) return(0);
	i++;
	}
cookie_name=(char *)malloc(1000);
if(getenv("HTTP_COOKIE")==NULL || type==1) message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
nb_cookies=get_nb_cookies(getenv("HTTP_COOKIE"));
if(nb_cookies>0)
	{
	cookie_id=0;
	for(i=1;i<=nb_cookies;i++)
		{
		get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
		if(strcmp(cookie_name,"reflexemail")==0) cookie_id=i;
		}
	if(cookie_id!=0)
		{
		get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,nom);
		cookie_id=0;
		for(i=1;i<=nb_cookies;i++)
			{
			get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
			if(strcmp(cookie_name,"reflexmdp")==0) cookie_id=i;
			}
		if(cookie_id!=0)
			{
			get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,mdp);
			cookie_id=0;
			for(i=1;i<=nb_cookies;i++)
				{
				get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
				if(strcmp(cookie_name,"reflexcode")==0) cookie_id=i;
				}
			if(cookie_id!=0)
				{
				get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,code);
				return(-1);
				}
			}
		}
	}	
message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
}

int veille_au_grain4(char *envoi, int nb, char *nom, char *mdp, char *code, char *owner, int type)
{
int i,somme,nb_cookies,cookie_id;
char *cookie_name;

i=somme=0;
while(i<strlen(envoi))
	{
	if(envoi[i]=='=') somme++;
	if(somme==nb) return(0);
	i++;
	}
cookie_name=(char *)malloc(1000);
if(getenv("HTTP_COOKIE")==NULL || type==1) message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
nb_cookies=get_nb_cookies(getenv("HTTP_COOKIE"));
if(nb_cookies>0)
	{
	cookie_id=0;
	for(i=1;i<=nb_cookies;i++)
		{
		get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
		if(strcmp(cookie_name,"reflexemail")==0) cookie_id=i;
		}
	if(cookie_id!=0)
		{
		get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,nom);
		cookie_id=0;
		for(i=1;i<=nb_cookies;i++)
			{
			get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
			if(strcmp(cookie_name,"reflexmdp")==0) cookie_id=i;
			}
		if(cookie_id!=0)
			{
			get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,mdp);
			cookie_id=0;
			for(i=1;i<=nb_cookies;i++)
				{
				get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
				if(strcmp(cookie_name,"reflexcode")==0) cookie_id=i;
				}
			if(cookie_id!=0)
				{
				get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,code);
				cookie_id=0;
				for(i=1;i<=nb_cookies;i++)
					{
					get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
					if(strcmp(cookie_name,"reflexowner")==0) cookie_id=i;
					}
				if(cookie_id!=0)
					{
					get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,owner);
					return(-1);
					}
				}
			}
		}
	}	
message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
}

int veille_au_grain5(char *envoi, int nb, char *nom, char *mdp, char *code, char *owner, char *album,int type)
{
int i,somme,nb_cookies,cookie_id;
char *cookie_name;

i=somme=0;
while(i<strlen(envoi))
	{
	if(envoi[i]=='=') somme++;
	if(somme==nb) return(0);
	i++;
	}
cookie_name=(char *)malloc(1000);
if(getenv("HTTP_COOKIE")==NULL || type==1) message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
nb_cookies=get_nb_cookies(getenv("HTTP_COOKIE"));
if(nb_cookies>0)
	{
	cookie_id=0;
	for(i=1;i<=nb_cookies;i++)
		{
		get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
		if(strcmp(cookie_name,"reflexemail")==0) cookie_id=i;
		}
	if(cookie_id!=0)
		{
		get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,nom);
		cookie_id=0;
		for(i=1;i<=nb_cookies;i++)
			{
			get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
			if(strcmp(cookie_name,"reflexmdp")==0) cookie_id=i;
			}
		if(cookie_id!=0)
			{
			get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,mdp);
			cookie_id=0;
			for(i=1;i<=nb_cookies;i++)
				{
				get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
				if(strcmp(cookie_name,"reflexcode")==0) cookie_id=i;
				}
			if(cookie_id!=0)
				{
				get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,code);
				cookie_id=0;
				for(i=1;i<=nb_cookies;i++)
					{
					get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
					if(strcmp(cookie_name,"reflexowner")==0) cookie_id=i;
					}
				if(cookie_id!=0)
					{
					get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,owner);
					cookie_id=0;
					for(i=1;i<=nb_cookies;i++)
						{
						get_cookie_name(getenv("HTTP_COOKIE"),i,cookie_name);
						if(strcmp(cookie_name,"reflexalbum")==0) cookie_id=i;
						}
					if(cookie_id!=0)
						{
						get_cookie_value(getenv("HTTP_COOKIE"),cookie_id,album);
						return(-1);
						}
					}
				}
			}
		}
	}	
message_service("Oups.<br><br>Reflex étant hautement sécurisé, cette page ne peut pas être rafraichie. <br><br>Désolé, il va falloir se reconnecter<br><a href=\"/reflex/index.html\" class=\"ui-btn ui-btn-c ui-corner-all mc-top-margin-1-5\">Reconnexion</a>");
}