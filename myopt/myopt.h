#if !defined(H_MYOPT)
#define H_MYOPT
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

enum char_options {END_OF_OPTIONS = -1};

extern int 		opt_index;      
extern char *	opt_arg;    

extern int 		options(int argc, char *argv[], const char *legal);

// Long options

struct option {
    const char *name;
    int         has_arg;
    int        *flag;
    int         val;
};

#define no_argument			0
#define required_argument	1
#define optional_argument	2

extern
int options_l	(
				int argc, 
				char * const argv[],
				const char *legal,
				const struct option *longopts, 
				int *longindex
				);

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
#endif

