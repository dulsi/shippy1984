#include "font.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <fontconfig/fontconfig.h>
#endif

static TTF_Font *defaultfont = NULL;

char *FindFontPath(char *data_path)
{
#ifdef __linux__
  char *answer = strdup("/usr/share/fonts/liberation-sans/LiberationSans-Bold.ttf");
  FcFontSet	*fs;
  FcPattern   *pat;
  FcResult	result;
  if (!FcInit())
  {
    return answer;
  }
  pat = FcNameParse((FcChar8 *)"LiberationSans:bold");
  FcConfigSubstitute(0, pat, FcMatchPattern);
  FcDefaultSubstitute(pat);

  fs = FcFontSetCreate();
  FcPattern   *match;
  match = FcFontMatch(0, pat, &result);
  if (match)
  {
    FcChar8 *file;
    if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch)
    {
      free(answer);
      answer = strdup((const char *)file);
    }
    FcPatternDestroy(match);
  }
  FcFontSetDestroy(fs);
  FcPatternDestroy(pat);
  // Could another library be using fontconfig?
  //FcFini();
#else
  char *answer = (char *)malloc(sizeof(char) * strlen(data_path) + 50);
  sprintf(answer, "%sLiberationSans-Bold.ttf", data_path);
#endif
  return answer;
}

TTF_Font *GetDefaultFont(char *data_path)
{
  if (defaultfont == NULL)
  {
    char *filename = FindFontPath(data_path);
    defaultfont = TTF_OpenFont(filename, 13);
    if (defaultfont == NULL)
    {
      printf("Failed to load font %s\n", filename);
      exit(1);
    }
    free(filename);
  }
  return defaultfont;
}
