#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define MP_SIZE 2
#define P_MUL 10
#define ORDER_MIN 2
#define FLAG_COLOMBIANS_ONLY 1
#define FLAG_PLANARS_ONLY 2
#define FLAG_FIRST_ONLY 4
#define FLAG_CIRCULAR 8
#define FLAG_VERBOSE 16
#define PLANS_DEF 2

typedef struct {
	int step;
	int start;
	int end;
	int plan;
}
option_t;

typedef struct {
	int val;
	int plan;
}
number_t;

typedef struct node_s node_t;

struct node_s {
	int rows_n;
	node_t *column;
	option_t *option;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

static void usage(void);
static int dlx_run(int, int (*)(int), int (*)(int), void (*)(int), int);
static void mp_new(int []);
static void mp_inc(int []);
static int mp_eq_zero(const int []);
static void mp_print(const char *, const int []);
static int count_group_options_fn2_fn3(int (*)(int), int (*)(int));
static void set_column(node_t *, node_t *);
static void add_group_options_fn2_fn3(int (*)(int), int (*)(int));
static void add_group_option(int);
static int count_half_circular_group_options(int);
static int count_circular_group_options(int);
static void add_strict_half_group_options(int);
static int count_half_group_options(int);
static int count_group_options(int);
static void add_options(int, int, int);
static void set_option(int, int, int);
static int compare_options(const void *, const void *);
static void add_row_nodes(option_t *);
static void set_row_node(node_t *, option_t *, node_t *, node_t **);
static void link_left(node_t *, node_t *);
static void link_top(node_t *, node_t *);
static void dlx_search(void);
static void assign_row_with_conflicts(const node_t *);
static void assign_row(const node_t *);
static void cover_row_columns(const node_t *);
static void cover_column(node_t *);
static void cover_conflicts(const option_t *);
static void check_conflicts1(node_t *, node_t *, int, int);
static void check_conflicts2(node_t *, node_t *, const option_t *, int, int);
static int range_conflict(const option_t *, int, int);
static int range_inside(const option_t *, int, int);
static void cover_conflict(node_t *);
static void cover_node(node_t *);
static void assign_option(const option_t *);
static void set_number(number_t *, int, int);
static void uncover_row_columns(const node_t *);
static void uncover_column(node_t *);
static void uncover_row(node_t *);
static void uncover_node(node_t *);
static void print_number(const number_t *);
static void main_free(void);

static int order, intervals_n, range_inf, range_sup, hooks_n, numbers_n, columns_n, setting_colombians_only, setting_planars_only, setting_first_only, setting_circular, setting_verbose, sentinel, plans_n, p_max, p_len, cost[MP_SIZE], solutions_n[MP_SIZE];
static void (*assign_row_fn)(const node_t *);
static option_t *options_cur;
static number_t *numbers;
static node_t **tops, *nodes, **conflicts_cur, *column_first_group, *column_sentinel, *header, *row_node;

int main(void) {
	int groups_n, settings;
	unsigned time0;
	if (scanf("%d", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %d\n\n", ORDER_MIN);
		usage();
		return EXIT_FAILURE;
	}
	intervals_n = order-1;
	if (scanf("%d", &range_inf) != 1 || range_inf < 1) {
		fputs("Range inferior bound must be greater than or equal to 1\n\n", stderr);
		usage();
		return EXIT_FAILURE;
	}
	if (scanf("%d", &range_sup) != 1 || range_inf > range_sup) {
		fputs("Range superior bound must be greater than or equal to Range inferior bound\n\n", stderr);
		usage();
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1;
	if (scanf("%d", &hooks_n) != 1) {
		hooks_n = 0;
	}
	numbers_n = groups_n*order+hooks_n;
	columns_n = numbers_n+groups_n;
	if (scanf("%d", &settings) != 1) {
		settings = 0;
	}
	setting_colombians_only = settings & FLAG_COLOMBIANS_ONLY;
	setting_planars_only = settings & FLAG_PLANARS_ONLY;
	setting_first_only = settings & FLAG_FIRST_ONLY;
	setting_circular = settings & FLAG_CIRCULAR;
	setting_verbose = settings & FLAG_VERBOSE;
	if (setting_colombians_only) {
		if (scanf("%d", &sentinel) != 1) {
			sentinel = range_sup-1;
		}
		if (sentinel < range_inf || sentinel > range_sup) {
			fputs("Sentinel must lie between Range inferior bound and Range superior bound\n\n", stderr);
			usage();
			return EXIT_FAILURE;
		}
	}
	else {
		sentinel = range_inf;
	}
	if (setting_planars_only) {
		if (scanf("%d", &plans_n) != 1) {
			plans_n = PLANS_DEF;
		}
		if (plans_n < 1) {
			fputs("Number of plans must be greater than or equal to 1\n\n", stderr);
			usage();
			return EXIT_FAILURE;
		}
	}
	else {
		plans_n = 1;
	}
	printf("Order %d, Range [%d-%d]", order, range_inf, range_sup);
	if (hooks_n) {
		printf(", Hooks %d", hooks_n);
	}
	if (setting_colombians_only) {
		printf(", Colombians only (sentinel %d)", sentinel);
	}
	if (setting_planars_only) {
		printf(", Planars only (plans %d)", plans_n);
	}
	if (setting_first_only) {
		printf(", First only");
	}
	if (setting_circular) {
		printf(", Circular");
	}
	if (setting_verbose) {
		printf(", Verbose");
	}
	puts(".");
	fflush(stdout);
	for (p_max = 1, p_len = 0; p_max <= INT_MAX/P_MUL; p_max *= P_MUL, ++p_len);
	--p_max;
	mp_new(cost);
	mp_new(solutions_n);
	time0 = (unsigned)time(NULL);
	if (range_sup*intervals_n < numbers_n) {
		int prime, k;
		for (prime = ORDER_MIN; prime < order && order%prime; ++prime);
		k = (range_sup/prime-(range_inf-1)/prime)%prime;
		if (!k || (prime-k)*(order-1) <= hooks_n) {
			int j = (range_inf-1)%prime, power, i;
			for (power = prime; order%power == 0; power *= prime);
			for (i = 0; i < prime; ++i) {
				int val = k*prime+i;
				if (val < j) {
					val += power;
				}
				if (val-j == groups_n%power) {
					break;
				}
			}
			if (i < prime) {
				if (setting_verbose) {
					numbers = malloc(sizeof(number_t)*(size_t)numbers_n);
					if (!numbers) {
						fputs("Error allocating memory for numbers\n", stderr);
						fflush(stderr);
						return EXIT_FAILURE;
					}
				}
				tops = malloc(sizeof(node_t *)*(size_t)columns_n);
				if (!tops) {
					fputs("Error allocating memory for tops\n", stderr);
					fflush(stderr);
					if (setting_verbose) {
						free(numbers);
					}
					return EXIT_FAILURE;
				}
				assign_row_fn = setting_colombians_only || setting_planars_only ? assign_row_with_conflicts:assign_row;
				if (setting_circular) {
					if (!dlx_run(1, count_half_circular_group_options, count_circular_group_options, add_group_option, (numbers_n+(range_sup-1)*intervals_n-1-range_sup*intervals_n)/2)) {
						main_free();
						return EXIT_FAILURE;
					}
				}
				else {
					int sup_group_options_n = count_group_options(range_sup);
					if (!dlx_run(sup_group_options_n/2, count_group_options, count_group_options, add_strict_half_group_options, 0) || (sup_group_options_n%2 && (!setting_first_only || mp_eq_zero(solutions_n)) && !dlx_run(1, count_half_group_options, count_group_options, add_group_option, sup_group_options_n/2))) {
						main_free();
						return EXIT_FAILURE;
					}
				}
				main_free();
			}
		}
	}
	printf("Runtime %us\n", (unsigned)time(NULL)-time0);
	mp_print("Final cost", cost);
	mp_print("Solutions", solutions_n);
	fflush(stdout);
	return EXIT_SUCCESS;
}

static void usage(void) {
	fputs("Parameters read on standard input: order range_inf range_sup [ hooks_n ] [ settings ] [ sentinel ] [ plans_n ]\n\n", stderr);
	fprintf(stderr, "order must be greater than or equal to %d\n", ORDER_MIN);
	fputs("range_inf = range inferior bound: must be greater than or equal to 1\n", stderr);
	fputs("range_sup = range superior bound: must be greater than or equal to range_inf\n", stderr);
	fputs("hooks_n = number of hooks (default 0)\n", stderr);
	fputs("settings = sum of option settings (default 0)\n", stderr);
	fprintf(stderr, "- colombian solutions only = %d\n", FLAG_COLOMBIANS_ONLY);
	fprintf(stderr, "- planar solutions only = %d\n", FLAG_PLANARS_ONLY);
	fprintf(stderr, "- first solution only = %d\n", FLAG_FIRST_ONLY);
	fprintf(stderr, "- circular mode = %d\n", FLAG_CIRCULAR);
	fprintf(stderr, "- verbose mode = %d\n", FLAG_VERBOSE);
	fputs("sentinel: argument for the colombian variant (default range_sup-1)\n", stderr);
	fprintf(stderr, "plans_n = number of plans: argument for the planar variant (default %d)\n", PLANS_DEF);
	fflush(stderr);
}

static void mp_new(int mp[]) {
	int i;
	for (i = 0; i < MP_SIZE; ++i) {
		mp[i] = 0;
	}
}

static void mp_inc(int mp[]) {
	int i;
	for (i = 0; i < MP_SIZE && mp[i] == p_max; ++i) {
		mp[i] = 0;
	}
	if (i < MP_SIZE) {
		++mp[i];
	}
}

static int mp_eq_zero(const int mp[]) {
	int i;
	for (i = 0; i < MP_SIZE && !mp[i]; ++i);
	return i == MP_SIZE;
}

static void mp_print(const char *label, const int mp[]) {
	int i;
	for (i = MP_SIZE-1; i && !mp[i]; --i);
	printf("%s %d", label, mp[i]);
	for (--i; i+1; --i) {
		printf(",%0*d", p_len, mp[i]);
	}
	puts("");
}

static int dlx_run(int group_options_n1, int (*count_group_options_fn2)(int), int (*count_group_options_fn3)(int), void (*add_group_options_fn1)(int), int offset) {
	int group_options_n = group_options_n1, hook_options_n, i;
	option_t *options;
	node_t **conflicts;
	if (hooks_n) {
		if (range_inf < range_sup) {
			group_options_n += count_group_options_fn2_fn3(count_group_options_fn2, count_group_options_fn3);
		}
		hook_options_n = numbers_n;
	}
	else {
		if (range_inf+1 < range_sup) {
			group_options_n += count_group_options_fn2_fn3(count_group_options_fn2, count_group_options_fn3);
		}
		else if (range_inf+1 == range_sup) {
			int plans_max = 1;
			if (plans_max < plans_n) {
				++plans_max;
			}
			group_options_n += count_group_options_fn3(range_inf)*plans_max;
		}
		hook_options_n = 0;
	}
	options = malloc(sizeof(option_t)*(size_t)(group_options_n+hook_options_n));
	if (!options) {
		fputs("Error allocating memory for options\n", stderr);
		fflush(stderr);
		return 0;
	}
	nodes = malloc(sizeof(node_t)*(size_t)(columns_n+1+group_options_n*(order+1)+hook_options_n));
	if (!nodes) {
		fputs("Error allocating memory for nodes\n", stderr);
		fflush(stderr);
		free(options);
		return 0;
	}
	if (setting_colombians_only || setting_planars_only) {
		conflicts = malloc(sizeof(node_t *)*(size_t)group_options_n);
		if (!conflicts) {
			fputs("Error allocating memory for conflicts\n", stderr);
			fflush(stderr);
			free(nodes);
			free(options);
			return 0;
		}
		conflicts_cur = conflicts;
	}
	else {
		conflicts = NULL;
	}
	column_first_group = nodes+numbers_n;
	column_sentinel = column_first_group+sentinel-range_inf;
	header = nodes+columns_n;
	set_column(nodes, header);
	for (i = 0; i < columns_n; ++i) {
		set_column(nodes+i+1, nodes+i);
		tops[i] = nodes+i;
	}
	options_cur = options;
	row_node = nodes+columns_n+1;
	add_group_options_fn1(offset);
	if (hooks_n) {
		if (range_inf < range_sup) {
			add_group_options_fn2_fn3(count_group_options_fn2, count_group_options_fn3);
		}
	}
	else {
		if (range_inf+1 < range_sup) {
			add_group_options_fn2_fn3(count_group_options_fn2, count_group_options_fn3);
		}
		else if (range_inf+1 == range_sup) {
			add_options(count_group_options_fn3(range_inf), range_inf, 1);
			if (plans_n > 1) {
				add_options(count_group_options_fn3(range_inf), range_inf, 2);
			}
		}
	}
	qsort(options, (size_t)group_options_n, sizeof(option_t), compare_options);
	for (i = 0; i < group_options_n; ++i) {
		add_row_nodes(options+i);
	}
	for (i = 0; i < hook_options_n; ++i) {
		set_option(0, i, 0);
		set_row_node(nodes+i, options_cur, row_node, tops+i);
		++options_cur;
	}
	for (i = 0; i < columns_n; ++i) {
		link_top(nodes+i, tops[i]);
	}
	dlx_search();
	if (setting_colombians_only || setting_planars_only) {
		free(conflicts);
	}
	free(nodes);
	free(options);
	return 1;
}

static int count_group_options_fn2_fn3(int (*count_group_options_fn2)(int), int (*count_group_options_fn3)(int)) {
	int plans_max = 1, group_options_n, i;
	if (plans_max < plans_n) {
		++plans_max;
	}
	group_options_n = count_group_options_fn2(range_sup-1)*plans_max;
	for (i = range_sup-2; i >= range_inf; --i) {
		if (plans_max < plans_n) {
			++plans_max;
		}
		group_options_n += count_group_options_fn3(i)*plans_max;
	}
	return group_options_n;
}

static void set_column(node_t *column, node_t *left) {
	column->rows_n = 0;
	link_left(column, left);
}

static void add_group_options_fn2_fn3(int (*count_group_options_fn2)(int), int (*count_group_options_fn3)(int)) {
	int plans_max = 1, i;
	if (plans_max < plans_n) {
		++plans_max;
	}
	add_options(count_group_options_fn2(range_sup-1), range_sup-1, plans_max);
	for (i = range_sup-2; i >= range_inf; --i) {
		if (plans_max < plans_n) {
			++plans_max;
		}
		add_options(count_group_options_fn3(i), i, plans_max);
	}
}

static void add_group_option(int offset) {
	set_option(range_sup, offset, 0);
	++options_cur;
}

static int count_half_circular_group_options(int step) {
	int val = count_circular_group_options(step);
	return val/2+val%2;
}

static int count_circular_group_options(int step) {
	return numbers_n-step*0;
}

static void add_strict_half_group_options(int offset) {
	add_options((count_group_options(range_sup)-offset)/2, range_sup, 1);
}

static int count_half_group_options(int step) {
	int val = count_group_options(step);
	return val/2+val%2;
}

static int count_group_options(int step) {
	return numbers_n-step*intervals_n;
}

static void add_options(int options_n, int step, int plans_max) {
	int i;
	for (i = 0; i < options_n; ++i) {
		int j;
		for (j = 0; j < plans_max; ++j) {
			set_option(step, i, j);
			++options_cur;
		}
	}
}

static void set_option(int step, int start, int plan) {
	options_cur->step = step;
	options_cur->start = start;
	options_cur->end = start+step*intervals_n;
	options_cur->plan = plan;
}

static int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->step != option_b->step) {
		return option_b->step-option_a->step;
	}
	if (option_a->start != option_b->start) {
		return option_a->start-option_b->start;
	}
	return option_a->plan-option_b->plan;
}

static void add_row_nodes(option_t *option) {
	int number_pos, i;
	if (option->step < range_sup && option->step*order == numbers_n && option->end >= numbers_n) {
		return;
	}
	number_pos = option->start;
	set_row_node(nodes+number_pos, option, row_node+order, tops+number_pos);
	for (i = 1; i < order; ++i) {
		number_pos += option->step;
		if (number_pos >= numbers_n) {
			number_pos -= numbers_n;
		}
		set_row_node(nodes+number_pos, option, row_node-1, tops+number_pos);
	}
	set_row_node(column_first_group+option->step-range_inf, option, row_node-1, tops+numbers_n+option->step-range_inf);
}

static void set_row_node(node_t *column, option_t *option, node_t *left, node_t **top) {
	++column->rows_n;
	row_node->column = column;
	row_node->option = option;
	link_left(row_node, left);
	link_top(row_node, *top);
	*top = row_node++;
}

static void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

static void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

static void dlx_search(void) {
	mp_inc(cost);
	if (header->right != header) {
		node_t *column_min = header->right, *column, *bottom;
		if (!column_min->rows_n) {
			return;
		}
		for (column = column_min->right; column != header; column = column->right) {
			if (column->rows_n < column_min->rows_n) {
				if (!column->rows_n) {
					return;
				}
				column_min = column;
			}
		}
		cover_column(column_min);
		if (setting_first_only) {
			int half_rows_min = column_min->rows_n/2+column_min->rows_n%2, i;
			node_t *top;
			for (i = 1, top = column_min->bottom; i < half_rows_min; ++i, top = top->bottom);
			bottom = top->bottom;
			if (column_min->rows_n%2) {
				assign_row_fn(top);
				top = top->top;
			}
			for (; top != column_min && mp_eq_zero(solutions_n); top = top->top, bottom = bottom->bottom) {
				assign_row_fn(top);
				if (mp_eq_zero(solutions_n)) {
					assign_row_fn(bottom);
				}
			}
		}
		else {
			for (bottom = column_min->bottom; bottom != column_min; bottom = bottom->bottom) {
				assign_row_fn(bottom);
			}
		}
		uncover_column(column_min);
		return;
	}
	mp_inc(solutions_n);
	if (setting_verbose) {
		int i;
		mp_print("Cost", cost);
		print_number(numbers);
		for (i = 1; i < numbers_n; ++i) {
			putchar(' ');
			print_number(numbers+i);
		}
		puts("");
		fflush(stdout);
	}
}

static void assign_row_with_conflicts(const node_t *row) {
	node_t **conflicts_bak = conflicts_cur;
	cover_row_columns(row);
	cover_conflicts(row->option);
	assign_option(row->option);
	dlx_search();
	while (conflicts_cur > conflicts_bak) {
		--conflicts_cur;
		uncover_row(*conflicts_cur);
	}
	uncover_row_columns(row);
}

static void assign_row(const node_t *row) {
	cover_row_columns(row);
	assign_option(row->option);
	dlx_search();
	uncover_row_columns(row);
}

static void cover_row_columns(const node_t *row) {
	node_t *node;
	for (node = row->right; node != row; node = node->right) {
		cover_column(node->column);
	}
}

static void cover_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->bottom; row != column; row = row->bottom) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			cover_node(node);
		}
	}
}

static void cover_conflicts(const option_t *option) {
	if (!option->step) {
		return;
	}
	if (setting_colombians_only) {
		if (option->step == sentinel) {
			node_t *column_inf = nodes+option->start, *column_sup, *column;
			for (column = header->right; column < column_inf; column = column->right);
			if (option->end < numbers_n) {
				column_sup = nodes+option->end;
			}
			else {
				check_conflicts1(column, column_first_group, option->start, option->end);
				column = header->right;
				column_sup = nodes+option->end-numbers_n;
			}
			check_conflicts1(column, column_sup, option->start, option->end);
		}
		else if (option->step < sentinel) {
			node_t *column;
			for (column = header->left; column != header && column > column_sentinel; column = column->left);
			if (column == column_sentinel) {
				node_t *row;
				for (row = column->bottom; row != column; row = row->bottom) {
					if (range_inside(row->option, option->start, option->end)) {
						cover_conflict(row);
					}
				}
			}
		}
	}
	if (setting_planars_only) {
		int inf = option->start, i;
		node_t *column_inf = nodes+inf, *column;
		for (column = header->right; column < column_inf; column = column->right);
		for (i = 1; i < order; ++i) {
			int sup;
			node_t *column_sup;
			if (inf < numbers_n) {
				sup = inf+option->step;
				if (sup < numbers_n) {
					column_sup = nodes+sup;
				}
				else {
					check_conflicts2(column, column_first_group, option, inf, sup);
					column = header->right;
					column_sup = nodes+sup-numbers_n;
				}
			}
			else {
				inf -= numbers_n;
				column_inf = nodes+inf;
				sup = inf+option->step;
				for (column = header->right; column < column_inf; column = column->right);
				column_sup = nodes+sup;
			}
			check_conflicts2(column, column_sup, option, inf, sup);
			inf = sup;
		}
	}
}

static void check_conflicts1(node_t *column, node_t *column_sup, int inf, int sup) {
	while (column < column_sup) {
		node_t *row;
		for (row = column->bottom; row != column; row = row->bottom) {
			if (!range_conflict(row->option, inf, sup)) {
				cover_conflict(row);
			}
		}
		column = column->right;
	}
}

static void check_conflicts2(node_t *column, node_t *column_sup, const option_t * option, int inf, int sup) {
	while (column < column_sup) {
		node_t *row;
		for (row = column->bottom; row != column; row = row->bottom) {
			if (row->option->plan == option->plan && range_conflict(row->option, inf, sup)) {
				cover_conflict(row);
			}
		}
		column = column->right;
	}
}

static int range_conflict(const option_t *option, int inf, int sup) {
	return option->start < inf || option->end > sup;
}

static int range_inside(const option_t *option, int inf, int sup) {
	return option->start < inf && option->end > sup;
}

static void cover_conflict(node_t *row) {
	node_t *node;
	cover_node(row);
	for (node = row->right; node != row; node = node->right) {
		cover_node(node);
	}
	*conflicts_cur = row;
	++conflicts_cur;
}

static void cover_node(node_t *node) {
	--node->column->rows_n;
	node->bottom->top = node->top;
	node->top->bottom = node->bottom;
}

static void assign_option(const option_t *option) {
	if (setting_verbose) {
		int number_pos = option->start;
		set_number(numbers+number_pos, option->step, option->plan);
		if (option->step) {
			int i;
			for (i = 1; i < order; ++i) {
				number_pos += option->step;
				if (number_pos >= numbers_n) {
					number_pos -= numbers_n;
				}
				set_number(numbers+number_pos, option->step, option->plan);
			}
		}
	}
}

static void set_number(number_t *number, int val, int plan) {
	number->val = val;
	number->plan = plan;
}

static void uncover_row_columns(const node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_column(node->column);
	}
}

static void uncover_column(node_t *column) {
	node_t *row;
	for (row = column->top; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			uncover_node(node);
		}
	}
	column->left->right = column;
	column->right->left = column;
}

static void uncover_row(node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_node(node);
	}
	uncover_node(row);
}

static void uncover_node(node_t *node) {
	node->top->bottom = node;
	node->bottom->top = node;
	++node->column->rows_n;
}

static void print_number(const number_t *number) {
	printf("%d", number->val);
	if (plans_n > 1) {
		printf("-%d", number->plan);
	}
}

static void main_free(void) {
	free(tops);
	if (setting_verbose) {
		free(numbers);
	}
}
