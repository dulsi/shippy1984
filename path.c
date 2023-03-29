#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
