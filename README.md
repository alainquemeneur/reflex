# reflex

Reflex : Réseau social auto-hébergé de partage de photos

Reflex est un réseau social de type webapp auto-hébergé sur un serveur Linux, orienté smartphones, permettant de partager ses photos avec ses amis. Il permet de créer des albums (privés ou publics), d'y uploader ses photos, de suivre sur un mur les dernières photos uploadées par ses amis (on peut suivre d'autres utilisateurs), ou les photos postées par la communauté fréquentant le serveur. On peut également liker les photos.

Le logiciel ne gère ni l'upload de vidéos, ni les commentaires sur les photos. C'est juste du partage et de la sauvegarde en ligne de photos. Par contre, il peut s'installer comme application (webapp) sur le bureau d'un smartphone ou même d'un ordinateur.

J'ai programmé ce logiciel pour répondre à mes besoins.

Techniquement, il s'agit d'une webapp installable n'importe quel OS (Windows, Linux, Mac, iOS, Android), mais prévue pour fonctionner surtout sous iOS ou Android. Le front end est programmé en HTML 5 + toolkit javascript JQuery Mobile. Le back end est constitué de scripts CGI écrits en langage C, traitant les données en interface avec une base de données MySQL ou MariaDB.

Ce programme ne peut fonctionner sans qrencode (pour la génération des QR codes 2FA), ni convert (pour la génération des miniatures)

<strong><u>Installation</u></strong>

Sur un serveur Linux, installer NginX, activer le https sur votre domaine, et <a href="https://techexpert.tips/nginx/nginx-shell-script-cgi/">activez les scripts CGI</a> sur votre serveur NginX

Installez reflex dans /home/votre_repertoire/reflex

Intégrez votre reflex dans votre serveur web :
sudo ln -s /home/votre_repertoire/reflex /var/www/html/reflex

Installez JQuery Mobile dans /var/www/html/jq :
sudo cp jq.tgz /var/www/html
cd /var/www/html
sudo tar -zxvf ./jq.tgz

Désormais votre webapp est accessible à l'URL : https://votre_serveur/reflex

Installez qrencode pour être en mesure de générer les QR codes nécessaires pour le 2FA Google Authenticator:
sudo apt-get install qrencode

Assurez vous des bons droits sur les répertoires avatar qr storage:
cd /home/votre_repertoire/reflex
chmod 777 avatar qr storage

Il faut maintenant préparer la base de données MySQL :
cd /home/votre_repertoire/reflex/build
mysql -u root -p mysql
create database reflex;
create user 'reflex'@'localhost' identified by 'votre_mot_de_passe';
grant all privileges on reflex.* to 'reflex'@'localhost';
quit

mysql -u reflex -p reflex
source ./install_mysql_tables.sql;
quit

Avant de compiler les scripts CGI, il faut leur indiquer le mot de passe choisi pour l'utilisateur reflex dans MySQL ainsi que l'URL du serveur sur Internet:
cd /home/votre_repertoire/reflex/src
Renseigner le mot de passe MySQL que vous avez choisi dans password.h

Il faut maintenant compiler les scripts CGI :
sudo apt install libmysqlclient-dev imagemagick-6.q16
cd /home/votre_repertoire/reflex/src
sudo mkdir /usr/lib/cg-bin
chmod a+x compile*
sudo ./compile.mysql

Si vous utilisez MariaDB plutôt que MySQL compilez à la place par :
sudo ./compile.mariadb

Une fois que les scripts sont compilés et apparus dans /usr/lib/cgi-bin, il faut effacer par sécurité tous les fichiers présents dans /home/votre_repertoire/reflex/src :
cd /home/votre_repertoire/reflex/src
rm * (vérifiez bien d'être dans le bon répertoire avant de taper cela !!!)

Connectez-vous à https://votre_serveur/reflex et utilisez la webapp