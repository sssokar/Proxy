#include <proxy.h>

static long	___CountWords(char *s, uint8_t c)
{
	long	nb = 0, i = 0;


	while (*(s+i))
	{
		if (*(s+i) != c) {
			++nb;
			while (*(s+i) && *(s+i) != c)
				++i;
		}
		else
			++i;
	}
	return (nb);
}

static char	**___ExtractWords(char *s, long n, uint8_t c)
{
	long i = 0, j = -1;

	char	**array = malloc(sizeof(char *)*(n+1));
	if (!array)
		return (NULL);

	while (*(s+i))
	{
		if (*(s+i) != c) {
			char	*tmp = NULL;
			tmp = (s+i);
			while (*(s+i) && *(s+i) != c)
			       ++i;
			if (*(s+i) == c) {
				*(s+i) = 0;
				++i;
			}
			array[++j] = strdup(tmp);
		}
		else
			++i;
	}
	array[++j] = NULL;
	return (array);
	}

char	**split(char *str, uint8_t c)
{
	long n = ___CountWords(str, c);

	if (n)
	{
		char 	**array = ___ExtractWords(str, n, c);
		return (array);
	}

	return (NULL);
}
