#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define mkdir(x,y) mkdir(x)
#endif

static char *data_path = NULL;

char *get_combine_path(char *dirs, char *subdir);

char *get_data_path()
{
  if (data_path == NULL)
  {
#ifdef WIN32
    data_path = strdup("./data/");
#else
#ifdef DATADIR
    char *data_home = getenv("XDG_DATA_DIRS");
    if (data_home)
    {
      data_path = get_combine_path(data_home, "shippy/");
    }
    else
    {
      data_home = strdup(DATADIR);
      data_path = get_combine_path(data_home, "shippy/");
      free(data_home);
    }
#else
    data_path = strdup("./data/");
#endif
#endif
  }
  return data_path;
}

char *get_combine_path(char *dirs, char *subdir)
{
  char fname[1024];
  char *current = dirs;
  char *end = strchr(current, ':');
  while (current != NULL)
  {
    if (end)
    {
      strncpy(fname, current, end - current);
      fname[end - current] = 0;
      current = end + 1;
      end = strchr(current, ':');
    }
    else
    {
      strcpy(fname, current);
      current = end;
    }
    int len = strlen(fname);
    if (fname[len - 1] != '/')
    {
      fname[len] = '/';
      fname[len + 1] = 0;
    }
    strcat(fname, subdir);
    len = strlen(fname);
    strcat(fname, "splash2.bmp");
    FILE *f = fopen(fname, "rb");
    if (f != NULL)
    {
      fclose(f);
      char *result = (char *)malloc(sizeof(char) * (len + 1));
      strncpy(result, fname, len);
      result[len] = 0;
      return result;
    }
  }
  return strdup("data/");
}

char *get_user_path()
{
  static char userPath[255] = "";
  struct stat statbuf;
  if (userPath[0] == 0) {
#ifdef WIN32
    strcpy(userPath, "save/");
#else
    //Temp variable that is used to prevent NULL assignement.
    char* env;

    //First get the $XDG_DATA_HOME env var.
    env=getenv("XDG_DATA_HOME");
    //If it's null set userPath to $HOME/.local/share/.
    if(env!=NULL){
      strcpy(userPath, env);
    }
    else {
      strcpy(userPath, getenv("HOME"));
      strcat(userPath, "/.local/share");
    }
    strcat(userPath, "/shippy/");
#endif
    if (0 != stat(userPath, &statbuf))
    {
      int len = strlen(userPath);
      for (int i = 1; i < len; i++)
      {
        if (userPath[i] == '/')
        {
          userPath[i] = 0;
          if (0 != stat(userPath, &statbuf))
          {
            mkdir(userPath, S_IRWXU);
          }
          userPath[i] = '/';
        }
      }
    }
  }
  return userPath;
}
