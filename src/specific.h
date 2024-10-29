#define STORAGE_DIR "/var/www/html/reflex/storage"
#define STORAGE_WEB "/reflex/storage"
#define NB_PHOTOS_MAX 25

struct photo
{
char compte[100],album[100],digest[100],horodatage[100],public[10];
int jaimes,jaime,blacklisted;
};

struct blacklist
{
char blacklister[100];
};

struct liker
{
char compte[100];
};

struct followed
{
char followed[100];
char follower[100];
};