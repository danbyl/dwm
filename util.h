/* See LICENSE file for copyright and license details. */

#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))
#define LENGTH(X)               (sizeof (X) / sizeof (X)[0])
#define LOG(...)                (log_msg(__func__, __VA_ARGS__))

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
__attribute__((format (printf, 2, 3))) void log_msg(const char *func, const char *fmt, ...);
