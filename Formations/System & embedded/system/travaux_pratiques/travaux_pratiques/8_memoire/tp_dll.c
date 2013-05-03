#include <stdio.h>
#include <dlfcn.h>

int main(void)
{
   const char *dlsym_error;
   typedef double (*cos_t)(double);
   double (*sin)(double);

   void* handle = dlopen("libm.so", RTLD_LAZY);
   if (!handle) {
      fprintf(stderr, "Cannot open library: %s\n", dlerror());
      return 1;
   }

   dlerror();        // reset errors
   cos_t cos = (cos_t) dlsym(handle, "cos");
   dlsym_error = dlerror();
   if (dlsym_error) {
      fprintf(stderr, "Cannot load symbol 'cos': %s\n", dlsym_error);
      dlclose(handle);
      return 1;
   }

   dlerror();        // reset errors
   *(double **)(&sin) = dlsym(handle, "sin");
   dlsym_error = dlerror();
   if (dlsym_error) {
      fprintf(stderr, "Cannot load symbol 'sin': %s\n", dlsym_error);
      dlclose(handle);
      return 1;
   }

   printf("cos 1 is %f\n", cos(1));
   printf("sin 1 is %f\n", sin(1));

   dlclose(handle);

   return 0;
}
