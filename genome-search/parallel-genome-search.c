/* Search a genome.
 */

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

int verbose = 0;				/* Output more info at run time? */
pthread_mutex_t matches_mutex;
//int matches = 0;

/* Helpful constants */
#define ONE_MEGA (long)(1024 * 1024)
#define ONE_GIGA (long)(ONE_MEGA * 1024)
#define ONE_BILLION (double)1000000000.0

const int line_buffer_length = 1024;

/* **************** FASTA ADT **************** */
/* Genome data files .. https://en.wikipedia.org/wiki/FASTA_format */

typedef struct {
  char *sequence;				/* Entire sequence */
  char *seq_ptr;				/* Next location to store data */
  long max_length;				/* Max length allocated */
  long cur_length;				/* Current length */
} fasta_t;

typedef struct {
    int tid;
    int fasta_max_length;
    int num_threads;
    int local_matches;
    char *pattern;
    fasta_t *fasta;
} thread_args_t;

/* Create a FASTA object; allocates memory on the heap */
fasta_t *
fasta_create(long max_length)
{
  if (verbose) {
	printf("Allocate %ld bytes\n", max_length);
  }
  fasta_t *new = malloc(sizeof(fasta_t));
  new->sequence = new->seq_ptr = malloc(max_length);
  new->max_length = max_length;
  new->cur_length = 0;
  return new;
}

/* Destroy a FASTA object; deallocates memory. */
void
fasta_destroy(fasta_t *old)
{
  free(old->sequence);
  free(old);
}

/* Read a FASTA file into a FASTA structure. Can be called multiple times and
 * will append new data to whatever is already in existing structure. For
 * example, can read multiple chromosome files into a single FASTA
 * structure. Will crater on attempts to read more data from file than will fit
 * in allocated FASTA structure. Works with both .gz and flat text files (but
 * prefer the zipped version to save disk space!).
 */
void
fasta_read_file(char *file_name, fasta_t *fasta)
{
  gzFile gzfp = gzopen(file_name, "rb");
  if (!gzfp) {
	fprintf(stderr, "Can't open '%s' for reading\n", file_name);
	exit(1);
  }
  printf(" LOADING %s\n", file_name);
  
  /* Reads one line at a time from the FASTA file. */
  char line_buffer[line_buffer_length];
  int lines_kept = 0;
  int lines_skipped = 0;
  while ((gzgets(gzfp, line_buffer, line_buffer_length) != NULL)) {
	char *line_ptr = line_buffer;
	if (line_buffer[0] == '>') {
	  /* Line contains text annotation; skip it. */
	  lines_skipped++;
	} else {
	  /* Valid data; copy all ACGT data from line buffer. */
	  lines_kept++;
	  while (*line_ptr != '\n') {
		*fasta->seq_ptr++ = *line_ptr++;
		fasta->cur_length++;
	  }
	}
	if (fasta->cur_length + line_buffer_length > fasta->max_length) {
	  fprintf(stderr, "Read %ld bytes; fasta buffer too small (%ld bytes)\n",
			  fasta->cur_length, fasta->max_length);
	  exit(1);
	}
  }

  if (verbose) {
	printf("%s: %d lines skipped, %d lines kept, %ld total bytes\n",
		   file_name, lines_skipped, lines_kept, fasta->cur_length);
  }
  
  gzclose(gzfp);
}

/* Return the current time. */
double
now(void)
{
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);
  return current_time.tv_sec + (current_time.tv_nsec / ONE_BILLION);
}

/* Return the smaller of two pointers. */
const char *
min(const char *a, const char *b)
{
  return a < b ? a : b;
}

/* Return the larger of two pointers. */
const char *
max(const char *a, const char *b)
{
  return a > b ? a : b;
}

/* Clamp 'value' to be in the range ['low' .. 'high'] */
const char *
clamp(const char *value, const char *low, const char *high)
{
  if (low >= high) {
	fprintf(stderr, "Bogus bounds [%p, %p]\n", low, high);
	exit(1);
  }

  return min(max(value, low), high);
}

/* Print 'n' padding characters to pretty-up the output. */
void
print_padding(const int n)
{
  for (int i = 0;  i < n;  i++) {
	printf(" ");
  }
}

/* Print 'length' bytes starting at 'current' from within the current data in
 * a FASTA structure. Also prints 'padding_bytes' bytes before and after the
 * range of values for context, as well as the offset into the sequence. Takes
 * care not to blow past either end of the sequence data in the FASTA
 * structure.
 */
void
bytes_around(fasta_t *fasta, char *current, int length)
{
  const int padding_bytes = 8;
  const char *fasta_last = fasta->sequence + fasta->cur_length;

  const char *first = clamp(current - padding_bytes, fasta->sequence, fasta_last);
  const char *last = clamp(current + length + padding_bytes, fasta->sequence, fasta_last);

  print_padding(padding_bytes - (current - first));

  for (const char *p = first;  p < last;  p++) {
	if (p == current) {
	  printf("[");
	}
	printf("%c", *p);
	if (p == current + length - 1) {
	  printf("]");
	}
  }

  print_padding(padding_bytes - (last - (current + length)));
  printf("%15ld\n", current - fasta->sequence);
}

/* Search for all occurrences of 'pattern' in the FASTA structure. Returns the
 * number of times the pattern occurs. Occurrences can overlap (e.g., "ACTACT"
 * matches twice in "ACTACTACT". Returns the number of occurrences. Also tracks
 * the time taken by the matching and reports the results to stdout.
 */
int 
match(char *pattern, fasta_t *fasta)
{
  int pattern_length = strlen(pattern);
  char *seq_ptr = fasta->sequence;
  char *last_match_location = fasta->sequence + fasta->cur_length - pattern_length;
  long match_count = 0;
  long trial_count = 0;

  double start_time = now();

  while (seq_ptr <= last_match_location) {
	trial_count++;
	if (strncmp(seq_ptr, pattern, pattern_length) == 0) {
	  if (verbose) {
		bytes_around(fasta, seq_ptr, pattern_length);
	  }
	  match_count++;
	}
	seq_ptr++;
  }

  printf("    TOOK %5.3f seconds\n", now() - start_time);
  printf("   TRIED %e matches\n", (double)trial_count);

  return match_count;
}

void
check_thread_rtn(char *msge, int rtn) {
  if (rtn) {
    fprintf(stderr, "ERROR: %s (%d)\n", msge,rtn);
    exit(1);
  }
}

void *
parallel_match(void *thread_args)
{
    thread_args_t *args = (thread_args_t *) thread_args;
    int pattern_length = strlen(args->pattern);
    char *seq_ptr = args->fasta->sequence + args->tid;
    char *last_match_location = args->fasta->sequence + args->fasta->cur_length - pattern_length;

    while(seq_ptr <= last_match_location) {
        if (strncmp(seq_ptr, args->pattern, pattern_length) == 0) {
            args->local_matches++;
        }
        seq_ptr += args->num_threads;
    }


    return (void *)NULL;
}

/* Print a usage message and exit. */
void
usage(char *prog_name)
{
  fprintf(stderr, "%s: [-v] -n <T> -b <B>|-m <MB>|-g <GB> -p <pattern> <fastafile>...\n", prog_name);
  fprintf(stderr, "  -v           enable verbose output\n");
  fprintf(stderr, "  -n <T>       use <T> threads to search for pattern\n");
  fprintf(stderr, "  -b <B>       allocate <B> bytes for FASTA data\n");
  fprintf(stderr, "  -m <MB>      allocate <MB> megabytes for FASTA data\n");
  fprintf(stderr, "  -g <GB>      allocate <GB> gigabytes for FASTA data\n");
  fprintf(stderr, "  -p <pattern> pattern for search [required]\n");
  fprintf(stderr, "  -h, -?       print this help and exit\n");
  fprintf(stderr, "One of -b, -m, or -g must be provided");
  fprintf(stderr, "One or more <fastafile> must appear; can be text or .gz file\n");
  fprintf(stderr, "If multiple <fastafile>s, will be concatenated and searched\n");
  exit(1);
}

int
main(int argc, char **argv)
{
  char *prog_name = argv[0];
  long fasta_max_length = 0;
  char *pattern = NULL;
  long num_threads = 0;

  /* Process command-line arguments; see 'man 3 getopt'. */
  int ch;
  while ((ch = getopt(argc, argv, "n:b:hm:g:p:v")) != -1) {
	switch (ch) {
	case 'n':
	  num_threads = atol(optarg);
	  break;
	case 'b':
	  fasta_max_length = atol(optarg);
	  break;
	case 'g':
	  fasta_max_length = atol(optarg) * ONE_GIGA;
	  break;
	case 'm':
	  fasta_max_length = atol(optarg) * ONE_MEGA;
	  break;
	case 'p':
	  pattern = optarg;
	  break;
	case 'v':
	  verbose = 1;
	  break;
	case 'h':
	case '?':
	default:
	  usage(prog_name);
	}
  }
  argc -= optind;
  argv += optind;

  printf("NUM THREADS: %ld\n", num_threads);

  if (fasta_max_length == 0 || pattern == NULL) {
	usage(prog_name);
  }

  /* Create FASTA structure with the given length. */
  fasta_t *fasta = fasta_create(fasta_max_length);

  /* For each <fastafile> argument, read its data into the FASTA structure. */
  for (int idx = 0;  idx < argc;  idx++) {
	fasta_read_file(argv[idx], fasta);
  }

  int rtn = pthread_mutex_init(&matches_mutex, NULL);
  check_thread_rtn("mutex init", rtn);


  pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
  thread_args_t *thread_args = malloc(sizeof(thread_args_t) * num_threads);

  printf("MATCHING ...\n");

  double start_time = now();
  int matches = 0;
  for (int i = 0;  i < num_threads;  i++) {
      thread_args[i].tid = i;
      thread_args[i].pattern = pattern;
      thread_args[i].fasta = fasta;
      thread_args[i].num_threads = num_threads;
      thread_args[i].local_matches = 0;
      rtn = pthread_create(&threads[i], NULL, parallel_match, &thread_args[i]);
      check_thread_rtn("create", rtn);
  }

  for (int i = 0;  i < num_threads;  i++) {
      rtn = pthread_join(threads[i], NULL);
      matches += thread_args[i].local_matches;
      check_thread_rtn("join", rtn);
  }
  printf("    TOOK %5.3f seconds\n", now() - start_time);

  printf("PATTERN %s\n", pattern);
  printf("   MATCH %d time%s\n", matches, matches == 1 ? "" : "s");

  /* Clean up and be done. */
  fasta_destroy(fasta);
  free(threads);
  free(thread_args);
  exit(0);
}
