#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define MP_SIZE 2
#define P_MUL 10U
#define ORDER_MIN 2U
#define FLAG_COLOMBIANS_ONLY 1
#define FLAG_PLANARS_ONLY 2
#define FLAG_FIRST_ONLY 4
#define FLAG_CIRCULAR 8
#define FLAG_VERBOSE 16
#define PLANS_DEF 2U

typedef struct {
	unsigned step;
	unsigned start;
	unsigned end;
	unsigned plan;
}
option_t;

typedef struct {
	unsigned val;
	unsigned plan;
}
number_t;

typedef struct node_s node_t;

struct node_s {
	unsigned rows_n;
	node_t *column;
	option_t *option;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

static void usage(void);
static int dlx_run(unsigned, unsigned (*)(unsigned), unsigned (*)(unsigned), void (*)(unsigned), unsigned);
static void mp_new(unsigned []);
static void mp_inc(unsigned []);
static int mp_eq_zero(const unsigned []);
static void mp_print(const char *, const unsigned []);
static unsigned set_group_options_fn2_fn3(unsigned (*)(unsigned), unsigned (*)(unsigned));
static void set_column(node_t *, node_t *);
static void add_group_options_fn2_fn3(unsigned (*)(unsigned), unsigned (*)(unsigned));
static void add_group_option(unsigned);
static unsigned set_group_half_circular_options(unsigned);
static unsigned set_group_circular_options(unsigned);
static void add_group_strict_half_options(unsigned);
static unsigned set_group_half_options(unsigned);
static unsigned set_group_all_options(unsigned);
static void add_options(unsigned, unsigned, unsigned);
static void set_option(unsigned, unsigned, unsigned);
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
static void check_conflicts1(node_t *, node_t *, unsigned, unsigned);
static void check_conflicts2(node_t *, node_t *, const option_t *, unsigned, unsigned);
static int range_conflict(const option_t *, unsigned, unsigned);
static int range_inside(const option_t *, unsigned, unsigned);
static void cover_conflict(node_t *);
static void cover_node(node_t *);
static void assign_option(const option_t *);
static void set_number(number_t *, unsigned, unsigned);
static void uncover_row_columns(const node_t *);
static void uncover_column(node_t *);
static void uncover_row(node_t *);
static void uncover_node(node_t *);
static void print_number(const number_t *);
static void main_free(void);

static int setting_planars_only, setting_colombians_only, setting_first_only, setting_circular, setting_verbose, p_len;
static unsigned order, intervals_n, range_inf, range_sup, hooks_n, numbers_n, columns_n, sentinel, plans_n, p_max, cost[MP_SIZE], solutions_n[MP_SIZE];
static void (*assign_row_fn)(const node_t *);
static option_t *options_cur;
static number_t *numbers;
static node_t **tops, *nodes, **conflicts_cur, *column_first_group, *column_sentinel, *header, *row_node;

int main(void) {
	int settings;
	unsigned groups_n, time0;
	if (scanf("%u", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %u\n\n", ORDER_MIN);
		usage();
		return EXIT_FAILURE;
	}
	intervals_n = order-1U;
	if (scanf("%u", &range_inf) != 1 || range_inf < 1U) {
		fputs("Range inferior bound must be greater than or equal to 1\n\n", stderr);
		usage();
		return EXIT_FAILURE;
	}
	if (scanf("%u", &range_sup) != 1 || range_inf > range_sup) {
		fputs("Range superior bound must be greater than or equal to Range inferior bound\n\n", stderr);
		usage();
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1U;
	if (scanf("%u", &hooks_n) != 1) {
		hooks_n = 0U;
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
		if (scanf("%u", &sentinel) != 1) {
			sentinel = range_sup-1U;
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
		if (scanf("%u", &plans_n) != 1) {
			plans_n = PLANS_DEF;
		}
		if (plans_n < 1U) {
			fputs("Number of plans must be greater than or equal to 1\n\n", stderr);
			usage();
			return EXIT_FAILURE;
		}
	}
	else {
		plans_n = 1U;
	}
	printf("Order %u, Range [%u-%u]", order, range_inf, range_sup);
	if (hooks_n) {
		printf(", Hooks %u", hooks_n);
	}
	if (setting_colombians_only) {
		printf(", Colombians only (sentinel %u)", sentinel);
	}
	if (setting_planars_only) {
		printf(", Planars only (plans %u)", plans_n);
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
	for (p_max = 1U, p_len = 0; p_max <= UINT_MAX/P_MUL; p_max *= P_MUL, ++p_len);
	--p_max;
	mp_new(cost);
	mp_new(solutions_n);
	time0 = (unsigned)time(NULL);
	if (range_sup*intervals_n < numbers_n) {
		unsigned prime, k;
		for (prime = ORDER_MIN; prime < order && order%prime; ++prime);
		k = (range_sup/prime-(range_inf-1U)/prime)%prime;
		if (!k || (prime-k)*(order-1U) <= hooks_n) {
			unsigned j = (range_inf-1U)%prime, power, i;
			for (power = prime; order%power == 0U; power *= prime);
			for (i = 0U; i < prime; ++i) {
				unsigned val = k*prime+i;
				if (val < j) {
					val += power;
				}
				if (val-j == groups_n%power) {
					break;
				}
			}
			if (i < prime) {
				if (setting_verbose) {
					numbers = malloc(sizeof(number_t)*numbers_n);
					if (!numbers) {
						fputs("Error allocating memory for numbers\n", stderr);
						fflush(stderr);
						return EXIT_FAILURE;
					}
				}
				tops = malloc(sizeof(node_t *)*columns_n);
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
					if (!dlx_run(1U, set_group_half_circular_options, set_group_circular_options, add_group_option, (numbers_n+(range_sup-1U)*intervals_n-1U-range_sup*intervals_n)/2U)) {
						main_free();
						return EXIT_FAILURE;
					}
				}
				else {
					unsigned group_sup_options_n = set_group_all_options(range_sup);
					if (!dlx_run(group_sup_options_n/2U, set_group_all_options, set_group_all_options, add_group_strict_half_options, 0U) || (group_sup_options_n%2U && (!setting_first_only || mp_eq_zero(solutions_n)) && !dlx_run(1U, set_group_half_options, set_group_all_options, add_group_option, group_sup_options_n/2U))) {
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
	fprintf(stderr, "order must be greater than or equal to %u\n", ORDER_MIN);
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
	fprintf(stderr, "plans_n = number of plans: argument for the planar variant (default %u)\n", PLANS_DEF);
	fflush(stderr);
}

static void mp_new(unsigned mp[]) {
	int i;
	for (i = 0; i < MP_SIZE; ++i) {
		mp[i] = 0U;
	}
}

static void mp_inc(unsigned mp[]) {
	int i;
	for (i = 0; i < MP_SIZE && mp[i] == p_max; ++i) {
		mp[i] = 0U;
	}
	if (i < MP_SIZE) {
		++mp[i];
	}
}

static int mp_eq_zero(const unsigned mp[]) {
	int i;
	for (i = 0; i < MP_SIZE && !mp[i]; ++i);
	return i == MP_SIZE;
}

static void mp_print(const char *label, const unsigned mp[]) {
	int i;
	for (i = MP_SIZE-1; i && !mp[i]; --i);
	printf("%s %u", label, mp[i]);
	for (--i; i+1; --i) {
		printf(",%0*u", p_len, mp[i]);
	}
	puts("");
}

static int dlx_run(unsigned group_options_n1, unsigned (*set_group_options_fn2)(unsigned), unsigned (*set_group_options_fn3)(unsigned), void (*add_group_options_fn1)(unsigned), unsigned offset) {
	unsigned group_options_n = group_options_n1, hook_options_n, i;
	option_t *options;
	node_t **conflicts;
	if (hooks_n) {
		if (range_inf < range_sup) {
			group_options_n += set_group_options_fn2_fn3(set_group_options_fn2, set_group_options_fn3);
		}
		hook_options_n = numbers_n;
	}
	else {
		if (range_inf+1U < range_sup) {
			group_options_n += set_group_options_fn2_fn3(set_group_options_fn2, set_group_options_fn3);
		}
		else if (range_inf+1U == range_sup) {
			unsigned plans_max = 1U;
			if (plans_max < plans_n) {
				++plans_max;
			}
			group_options_n += set_group_options_fn3(range_inf)*plans_max;
		}
		hook_options_n = 0U;
	}
	options = malloc(sizeof(option_t)*(group_options_n+hook_options_n));
	if (!options) {
		fputs("Error allocating memory for options\n", stderr);
		fflush(stderr);
		return 0;
	}
	nodes = malloc(sizeof(node_t)*(columns_n+1U+group_options_n*(order+1U)+hook_options_n));
	if (!nodes) {
		fputs("Error allocating memory for nodes\n", stderr);
		fflush(stderr);
		free(options);
		return 0;
	}
	if (setting_colombians_only || setting_planars_only) {
		conflicts = malloc(sizeof(node_t *)*group_options_n);
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
	for (i = 0U; i < columns_n; ++i) {
		set_column(nodes+i+1U, nodes+i);
		tops[i] = nodes+i;
	}
	options_cur = options;
	row_node = nodes+columns_n+1U;
	add_group_options_fn1(offset);
	if (hooks_n) {
		if (range_inf < range_sup) {
			add_group_options_fn2_fn3(set_group_options_fn2, set_group_options_fn3);
		}
	}
	else {
		if (range_inf+1U < range_sup) {
			add_group_options_fn2_fn3(set_group_options_fn2, set_group_options_fn3);
		}
		else if (range_inf+1U == range_sup) {
			add_options(set_group_options_fn3(range_inf), range_inf, 1U);
			if (plans_n > 1U) {
				add_options(set_group_options_fn3(range_inf), range_inf, 2U);
			}
		}
	}
	qsort(options, (size_t)group_options_n, sizeof(option_t), compare_options);
	for (i = 0U; i < group_options_n; ++i) {
		add_row_nodes(options+i);
	}
	for (i = 0U; i < hook_options_n; ++i) {
		set_option(0U, i, 0U);
		set_row_node(nodes+i, options_cur, row_node, tops+i);
		++options_cur;
	}
	for (i = 0U; i < columns_n; ++i) {
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

static unsigned set_group_options_fn2_fn3(unsigned (*set_group_options_fn2)(unsigned), unsigned (*set_group_options_fn3)(unsigned)) {
	unsigned plans_max = 1U, group_options_n, i;
	if (plans_max < plans_n) {
		++plans_max;
	}
	group_options_n = set_group_options_fn2(range_sup-1U)*plans_max;
	for (i = range_sup-2U; i >= range_inf; --i) {
		if (plans_max < plans_n) {
			++plans_max;
		}
		group_options_n += set_group_options_fn3(i)*plans_max;
	}
	return group_options_n;
}

static void set_column(node_t *column, node_t *left) {
	column->rows_n = 0U;
	link_left(column, left);
}

static void add_group_options_fn2_fn3(unsigned (*set_group_options_fn2)(unsigned), unsigned (*set_group_options_fn3)(unsigned)) {
	unsigned plans_max = 1U, i;
	if (plans_max < plans_n) {
		++plans_max;
	}
	add_options(set_group_options_fn2(range_sup-1U), range_sup-1U, plans_max);
	for (i = range_sup-2U; i >= range_inf; --i) {
		if (plans_max < plans_n) {
			++plans_max;
		}
		add_options(set_group_options_fn3(i), i, plans_max);
	}
}

static void add_group_option(unsigned offset) {
	set_option(range_sup, offset, 0U);
	++options_cur;
}

static unsigned set_group_half_circular_options(unsigned step) {
	unsigned val = set_group_circular_options(step);
	return val/2U+val%2U;
}

static unsigned set_group_circular_options(unsigned step) {
	return numbers_n-step*0U;
}

static void add_group_strict_half_options(unsigned offset) {
	add_options((set_group_all_options(range_sup)-offset)/2U, range_sup, 1U);
}

static unsigned set_group_half_options(unsigned step) {
	unsigned val = set_group_all_options(step);
	return val/2U+val%2U;
}

static unsigned set_group_all_options(unsigned step) {
	return numbers_n-step*intervals_n;
}

static void add_options(unsigned options_n, unsigned step, unsigned plans_max) {
	unsigned i;
	for (i = 0U; i < options_n; ++i) {
		unsigned j;
		for (j = 0U; j < plans_max; ++j) {
			set_option(step, i, j);
			++options_cur;
		}
	}
}

static void set_option(unsigned step, unsigned start, unsigned plan) {
	options_cur->step = step;
	options_cur->start = start;
	options_cur->end = start+step*intervals_n;
	options_cur->plan = plan;
}

static int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->step < option_b->step) {
		return 1;
	}
	if (option_a->step > option_b->step) {
		return -1;
	}
	if (option_a->start < option_b->start) {
		return -1;
	}
	if (option_a->start > option_b->start) {
		return 1;
	}
	if (option_a->plan < option_b->plan) {
		return -1;
	}
	return 1;
}

static void add_row_nodes(option_t *option) {
	unsigned number_pos, i;
	if (option->step < range_sup && option->step*order == numbers_n && option->end >= numbers_n) {
		return;
	}
	number_pos = option->start;
	set_row_node(nodes+number_pos, option, row_node+order, tops+number_pos);
	for (i = 1U; i < order; ++i) {
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
			unsigned half_rows_min = column_min->rows_n/2U+column_min->rows_n%2U, i;
			node_t *top;
			for (i = 1U, top = column_min->bottom; i < half_rows_min; ++i, top = top->bottom);
			bottom = top->bottom;
			if (column_min->rows_n%2U) {
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
		unsigned i;
		mp_print("Cost", cost);
		print_number(numbers);
		for (i = 1U; i < numbers_n; ++i) {
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
		unsigned inf = option->start, i;
		node_t *column_inf = nodes+inf, *column;
		for (column = header->right; column < column_inf; column = column->right);
		for (i = 1U; i < order; ++i) {
			unsigned sup;
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

static void check_conflicts1(node_t *column, node_t *column_sup, unsigned inf, unsigned sup) {
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

static void check_conflicts2(node_t *column, node_t *column_sup, const option_t * option, unsigned inf, unsigned sup) {
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

static int range_conflict(const option_t *option, unsigned inf, unsigned sup) {
	return option->start < inf || option->end > sup;
}

static int range_inside(const option_t *option, unsigned inf, unsigned sup) {
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
		unsigned number_pos = option->start;
		set_number(numbers+number_pos, option->step, option->plan);
		if (option->step) {
			unsigned i;
			for (i = 1U; i < order; ++i) {
				number_pos += option->step;
				if (number_pos >= numbers_n) {
					number_pos -= numbers_n;
				}
				set_number(numbers+number_pos, option->step, option->plan);
			}
		}
	}
}

static void set_number(number_t *number, unsigned val, unsigned plan) {
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
	printf("%u", number->val);
	if (plans_n > 1U) {
		printf("-%u", number->plan);
	}
}

static void main_free(void) {
	free(tops);
	if (setting_verbose) {
		free(numbers);
	}
}
