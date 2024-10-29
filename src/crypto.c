#include <string.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <ctype.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/sem.h>

void message_service(char *message)
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
   <div data-role=\"page\" data-theme=\"b\">\n\
      <div role=\"main\" class=\"ui-content\">\n\
   <p></p>\n\
   %s\n\
   </div>\n\
   <div data-role=\"footer\" data-position=\"fixed\" data-theme=\"b\">\n\
   <h4>Reflex - Partage de photos</h4>\n\
   </div>\n\
   </div>\n\
</body>\n\
</html>\n",message);
exit(0);
}