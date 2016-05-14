/* file: minunit.h */
#define mu_assert(name, message, test) do {\
  if (!(test)) {\
    printf("%s\t[Failed]\n", name);\
    return message;\
  } else {\
    printf("%s\t[Passed]\n", name);\
  }\
} while (0)

#define mu_run_test(test) do { char *message = test(); tests_run++; \
                               if (message) {tests_failed++; return message;} } while (0)
extern int tests_run;
extern int tests_failed;


