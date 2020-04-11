#include <stdio.h>
#include <stdlib.h>

#define ORDER_MIN 2UL
#define RANGE_INF_MIN 1UL
#define FLAG_PLANARS_ONLY 1
#define FLAG_FIRST_ONLY 2
#define FLAG_VERBOSE 4
#define SIDE_NORTH '+'
#define SIDE_SOUTH '-'
#define P_VAL_MAX 1000000000UL
#define P_DIGITS_MAX 9

typedef struct {
	unsigned long step;
	unsigned long start;
	unsigned long end;
	int side;
}
option_t;

typedef struct {
	int side;
	unsigned long value;
}
number_t;

typedef struct node_s node_t;

struct node_s {
	union {
		unsigned long rows_n;
		node_t *column;
	};
	option_t *option;
	node_t *left;
	node_t *right;
	node_t *top;
	node_t *bottom;
};

typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

int dlx_run(unsigned long (*)(unsigned long, unsigned long), unsigned long (*)(unsigned long), void (*)(unsigned long, unsigned long, int), void (*)(unsigned long, int), unsigned long, unsigned long, unsigned long);
void set_column(node_t *, node_t *);
void add_group_strict_half_options(unsigned long, unsigned long, int);
unsigned long set_group_strict_half_options(unsigned long, unsigned long);
void add_group_options(unsigned long, int);
unsigned long set_group_options(unsigned long);
void add_group_option(unsigned long, unsigned long, int);
unsigned long set_group_option(unsigned long, unsigned long);
void add_group_half_options(unsigned long, int);
unsigned long set_group_half_options(unsigned long);
void set_option(option_t *, unsigned long, unsigned long, int);
void add_row_nodes(option_t *);
void set_row_node(node_t *, option_t *, node_t *, node_t **);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(void);
void process_rows(node_t *, void (*)(node_t *));
void assign_row_planars_only(node_t *);
void assign_row(node_t *);
void cover_row_columns(node_t *);
void cover_column(node_t *);
void cover_conflicts(option_t *);
int range_conflict(option_t *, unsigned long, unsigned long);
void cover_node(node_t *);
void assign_option(option_t *);
void set_number(number_t *, int, unsigned long);
void uncover_row_columns(node_t *);
void uncover_column(node_t *);
void uncover_row(node_t *);
void uncover_node(node_t *);
int compare_options(const void *, const void *);
int mp_new(mp_t *);
void mp_print(const char *, mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(mp_t *, unsigned long);
void mp_free(mp_t *);

int setting_planars_only, setting_first_only, setting_verbose;
unsigned long order, intervals_n, range_inf, range_sup, hooks_n, numbers_n, columns_n;
option_t filler, *options_cur;
number_t *numbers;
node_t **tops, **top_first_group, *nodes, **conflicts_cur, *column_first_group, *header, *row_node;
mp_t cost, solutions_n;

int main(void) {
	int settings;
	unsigned long groups_n, group_sup_options_n;
	if (scanf("%lu", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %lu\n", ORDER_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	intervals_n = order-1UL;
	if (scanf("%lu", &range_inf) != 1 || range_inf < RANGE_INF_MIN) {
		fprintf(stderr, "Range inferior bound must be greater than or equal to %lu\n", RANGE_INF_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%lu", &range_sup) != 1 || range_sup < range_inf) {
		fprintf(stderr, "Range superior bound must be greater than or equal to Range inferior bound\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1UL;
	if (scanf("%lu", &hooks_n) != 1) {
		fprintf(stderr, "Error reading number of hooks\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	numbers_n = groups_n*order+hooks_n;
	columns_n = numbers_n+groups_n;
	if (scanf("%d", &settings) != 1) {
		fprintf(stderr, "Error reading setting flags\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	setting_planars_only = settings & FLAG_PLANARS_ONLY;
	setting_first_only = settings & FLAG_FIRST_ONLY;
	setting_verbose = settings & FLAG_VERBOSE;
	set_option(&filler, 0UL, numbers_n, SIDE_NORTH);
	if (setting_verbose) {
		numbers = malloc(sizeof(number_t)*numbers_n);
		if (!numbers) {
			fprintf(stderr, "Error allocating memory for numbers\n");
			fflush(stderr);
			return EXIT_FAILURE;
		}
	}
	tops = malloc(sizeof(node_t *)*columns_n);
	if (!tops) {
		fprintf(stderr, "Error allocating memory for tops\n");
		fflush(stderr);
		if (setting_verbose) {
			free(numbers);
		}
		return EXIT_FAILURE;
	}
	top_first_group = tops+numbers_n;
	if (!mp_new(&cost)) {
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		return EXIT_FAILURE;
	}
	if (!mp_new(&solutions_n)) {
		mp_free(&cost);
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		return EXIT_FAILURE;
	}
	group_sup_options_n = set_group_options(range_sup);
	if (!dlx_run(set_group_strict_half_options, set_group_options, add_group_strict_half_options, add_group_options, 0UL, 0UL, 0UL)) {
		mp_free(&solutions_n);
		mp_free(&cost);
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		return EXIT_FAILURE;
	}
	if (group_sup_options_n%2UL == 1UL && (!setting_first_only || mp_eq_val(&solutions_n, 0UL)) && !dlx_run(set_group_option, set_group_half_options, add_group_option, add_group_half_options, group_sup_options_n/2UL, 1UL, numbers_n/2UL+numbers_n%2UL)) {
		mp_free(&solutions_n);
		mp_free(&cost);
		free(tops);
		if (setting_verbose) {
			free(numbers);
		}
		return EXIT_FAILURE;
	}
	mp_print("Final cost", &cost);
	mp_print("Solutions", &solutions_n);
	fflush(stdout);
	mp_free(&solutions_n);
	mp_free(&cost);
	free(tops);
	if (setting_verbose) {
		free(numbers);
	}
	return EXIT_SUCCESS;
}

int dlx_run(unsigned long (*set_group_options_fn1)(unsigned long, unsigned long), unsigned long (*set_group_options_fn2)(unsigned long), void (*add_group_options_fn1)(unsigned long, unsigned long, int), void (*add_group_options_fn2)(unsigned long, int), unsigned long offset, unsigned long group_options_min, unsigned long hook_options_min) {
	unsigned long group_options_n = set_group_options_fn1(range_sup, offset), hook_options_n, nodes_n, i;
	option_t *options;
	node_t **conflicts;
	if (range_sup > range_inf) {
		if (setting_planars_only) {
			group_options_n += set_group_options_fn2(range_sup-1UL)*2UL;
			for (i = range_sup-2UL; i >= range_inf; i--) {
				group_options_n += set_group_options(i)*2UL;
			}
		}
		else {
			group_options_n += set_group_options_fn2(range_sup-1UL);
			for (i = range_sup-2UL; i >= range_inf; i--) {
				group_options_n += set_group_options(i);
			}
		}
	}
	if (hooks_n > 0UL) {
		if (group_options_n > group_options_min) {
			hook_options_n = numbers_n;
		}
		else {
			hook_options_n = hook_options_min;
		}
	}
	else {
		hook_options_n = 0UL;
	}
	options = malloc(sizeof(option_t)*(group_options_n+hook_options_n));
	if (!options) {
		fprintf(stderr, "Error allocating memory for options\n");
		fflush(stderr);
		return 0;
	}
	nodes_n = columns_n+1UL+group_options_n*(order+1UL)+hook_options_n;
	nodes = malloc(sizeof(node_t)*nodes_n);
	if (!nodes) {
		fprintf(stderr, "Error allocating memory for nodes\n");
		fflush(stderr);
		free(options);
		return 0;
	}
	if (setting_planars_only) {
		conflicts = malloc(sizeof(node_t *)*group_options_n);
		if (!conflicts) {
			fprintf(stderr, "Error allocating memory for conflicts\n");
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
	header = nodes+columns_n;
	set_column(nodes, header);
	for (i = 0UL; i < columns_n; i++) {
		set_column(nodes+i+1UL, nodes+i);
		tops[i] = nodes+i;
	}
	options_cur = options;
	row_node = nodes+columns_n+1UL;
	add_group_options_fn1(range_sup, offset, SIDE_NORTH);
	if (range_sup > range_inf) {
		add_group_options_fn2(range_sup-1UL, SIDE_NORTH);
		if (setting_planars_only) {
			add_group_options_fn2(range_sup-1UL, SIDE_SOUTH);
		}
		for (i = range_sup-2UL; i >= range_inf; i--) {
			add_group_options(i, SIDE_NORTH);
			if (setting_planars_only) {
				add_group_options(i, SIDE_SOUTH);
			}
		}
	}
	qsort(options, group_options_n, sizeof(option_t), compare_options);
	for (i = 0UL; i < group_options_n; i++) {
		add_row_nodes(options+i);
	}
	for (i = 0UL; i < hook_options_n; i++) {
		set_option(options_cur, 0UL, i, SIDE_NORTH);
		set_row_node(nodes+i, options_cur, row_node, tops+i);
		options_cur++;
	}
	for (i = 0UL; i < columns_n; i++) {
		link_top(nodes+i, tops[i]);
	}
	dlx_search();
	if (setting_planars_only) {
		free(conflicts);
	}
	free(nodes);
	free(options);
	return 1;
}

void set_column(node_t *column, node_t *left) {
	column->rows_n = 0UL;
	column->option = &filler;
	link_left(column, left);
}

void add_group_strict_half_options(unsigned long step, unsigned long offset, int side) {
	unsigned long group_options_n = set_group_strict_half_options(step, offset), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, side);
		options_cur++;
	}
}

unsigned long set_group_strict_half_options(unsigned long step, unsigned long offset) {
	return intervals_n*step+offset < numbers_n ? (numbers_n-intervals_n*step-offset)/2UL:0UL;
}

void add_group_options(unsigned long step, int side) {
	unsigned long group_options_n = set_group_options(step), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, side);
		options_cur++;
	}
}

unsigned long set_group_options(unsigned long step) {
	return intervals_n*step < numbers_n ? numbers_n-intervals_n*step:0UL;
}

void add_group_option(unsigned long step, unsigned long offset, int side) {
	unsigned long group_options_n = set_group_option(step, offset);
	if (group_options_n > 0UL) {
		set_option(options_cur, step, offset, side);
		options_cur++;
	}
}

unsigned long set_group_option(unsigned long step, unsigned long offset) {
	return intervals_n*step+offset < numbers_n ? 1UL:0UL;
}

void add_group_half_options(unsigned long step, int side) {
	unsigned long group_options_n = set_group_half_options(step), i;
	for (i = 0UL; i < group_options_n; i++) {
		set_option(options_cur, step, i, side);
		options_cur++;
	}
}

unsigned long set_group_half_options(unsigned long step) {
	return intervals_n*step < numbers_n ? (numbers_n-intervals_n*step)/2UL+(numbers_n-intervals_n*step)%2UL:0UL;
}

void set_option(option_t *option, unsigned long step, unsigned long start, int side) {
	option->step = step;
	option->start = start;
	option->end = start+step*intervals_n;
	option->side = side;
}

void add_row_nodes(option_t *option) {
	unsigned long i;
	set_row_node(nodes+option->start, option, row_node+order, tops+option->start);
	for (i = 1UL; i < order; i++) {
		set_row_node(nodes+option->start+i*option->step, option, row_node-1, tops+option->start+i*option->step);
	}
	set_row_node(column_first_group+option->step-range_inf, option, row_node-1, top_first_group+option->step-range_inf);
}

void set_row_node(node_t *column, option_t *option, node_t *left, node_t **top) {
	column->rows_n++;
	row_node->column = column;
	row_node->option = option;
	link_left(row_node, left);
	link_top(row_node, *top);
	*top = row_node++;
}

void link_left(node_t *node, node_t *left) {
	node->left = left;
	left->right = node;
}

void link_top(node_t *node, node_t *top) {
	node->top = top;
	top->bottom = node;
}

void dlx_search(void) {
	node_t *column_min, *column;
	if (!mp_inc(&cost)) {
		return;
	}
	if (header->right == header) {
		if (!mp_inc(&solutions_n)) {
			return;
		}
		if (setting_verbose) {
			unsigned long i;
			mp_print("Cost", &cost);
			if (setting_planars_only) {
				printf("%c%lu", numbers[0].side, numbers[0].value);
				for (i = 1UL; i < numbers_n; i++) {
					printf(" %c%lu", numbers[i].side, numbers[i].value);
				}
			}
			else {
				printf("%lu", numbers[0].value);
				for (i = 1UL; i < numbers_n; i++) {
					printf(" %lu", numbers[i].value);
				}
			}
			puts("");
			fflush(stdout);
		}
		return;
	}
	if (header->right->rows_n == 0UL) {
		return;
	}
	column_min = header->right;
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n < column_min->rows_n) {
			if (column->rows_n == 0UL) {
				return;
			}
			column_min = column;
		}
	}
	cover_column(column_min);
	if (setting_planars_only) {
		process_rows(column_min, assign_row_planars_only);
	}
	else {
		process_rows(column_min, assign_row);
	}
	uncover_column(column_min);
}

void process_rows(node_t *column_min, void (*assign_row_fn)(node_t *)) {
	if (setting_first_only) {
		unsigned long half_rows_min = column_min->rows_n/2UL+column_min->rows_n%2UL, i;
		node_t *middle, *top, *bottom;
		for (i = 0UL, middle = column_min; i < half_rows_min; i++, middle = middle->bottom);
		if (column_min->rows_n%2UL == 1UL) {
			if (mp_eq_val(&solutions_n, 0UL)) {
				assign_row_fn(middle);
			}
			for (top = middle->top, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
				assign_row_fn(top);
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row_fn(bottom);
				}
			}
		}
		else {
			for (top = middle, bottom = middle->bottom; top != column_min && mp_eq_val(&solutions_n, 0UL); top = top->top, bottom = bottom->bottom) {
				assign_row_fn(top);
				if (mp_eq_val(&solutions_n, 0UL)) {
					assign_row_fn(bottom);
				}
			}
		}
	}
	else {
		node_t *row;
		for (row = column_min->bottom; row != column_min; row = row->bottom) {
			assign_row_fn(row);
		}
	}
}

void assign_row_planars_only(node_t *row) {
	node_t **conflicts_bak = conflicts_cur;
	cover_row_columns(row);
	cover_conflicts(row->option);
	assign_option(row->option);
	dlx_search();
	while (conflicts_cur != conflicts_bak) {
		conflicts_cur--;
		uncover_row(*conflicts_cur);
	}
	uncover_row_columns(row);
}

void assign_row(node_t *row) {
	cover_row_columns(row);
	assign_option(row->option);
	dlx_search();
	uncover_row_columns(row);
}

void cover_row_columns(node_t *row) {
	node_t *node;
	for (node = row->right; node != row; node = node->right) {
		cover_column(node->column);
	}
}

void cover_column(node_t *column) {
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

void cover_conflicts(option_t *option) {
	unsigned long inf, sup, i;
	node_t *column_inf, *column;
	if (option->step == 0UL) {
		return;
	}
	inf = option->start;
	sup = inf+option->step;
	column_inf = nodes+inf;
	for (column = header->right; column < column_inf; column = column->right);
	for (i = 1UL; i < order; i++) {
		node_t *column_sup = nodes+sup;
		while (column < column_sup) {
			node_t *row;
			for (row = column->bottom; row != column; row = row->bottom) {
				if (row->option->side == option->side && range_conflict(row->option, inf, sup)) {
					node_t *node;
					cover_node(row);
					for (node = row->right; node != row; node = node->right) {
						cover_node(node);
					}
					*conflicts_cur = row;
					conflicts_cur++;
				}
			}
			column = column->right;
		}
		inf = sup;
		sup += option->step;
	}
}

int range_conflict(option_t *option, unsigned long inf, unsigned long sup) {
	return option->start < inf || option->end > sup;
}

void cover_node(node_t *node) {
	node->column->rows_n--;
	node->bottom->top = node->top;
	node->top->bottom = node->bottom;
}

void assign_option(option_t *option) {
	if (setting_verbose) {
		if (option->step > 0UL) {
			unsigned long i;
			for (i = 0UL; i < order; i++) {
				set_number(numbers+option->start+i*option->step, option->side, option->step);
			}
		}
		else {
			set_number(numbers+option->start, option->side, option->step);
		}
	}
}

void set_number(number_t *number, int side, unsigned long value) {
	number->side = side;
	number->value = value;
}

void uncover_row_columns(node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_column(node->column);
	}
}

void uncover_column(node_t *column) {
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

void uncover_row(node_t *row) {
	node_t *node;
	for (node = row->left; node != row; node = node->left) {
		uncover_node(node);
	}
	uncover_node(row);
}

void uncover_node(node_t *node) {
	node->top->bottom = node;
	node->bottom->top = node;
	node->column->rows_n++;
}

int compare_options(const void *a, const void *b) {
	const option_t *option_a = (const option_t *)a, *option_b = (const option_t *)b;
	if (option_a->start < option_b->start) {
		return -1;
	}
	if (option_a->start > option_b->start) {
		return 1;
	}
	if (option_a->step < option_b->step) {
		return 1;
	}
	if (option_a->step > option_b->step) {
		return -1;
	}
	return option_a->side-option_b->side;
}

int mp_new(mp_t *mp) {
	mp->m = 1UL;
	mp->p = calloc(1UL, sizeof(unsigned long));
	if (!mp->p) {
		fprintf(stderr, "Error allocating memory for mp->p\n");
		fflush(stderr);
		return 0;
	}
	return 1;
}

void mp_print(const char *title, mp_t *mp) {
	unsigned long i;
	printf("%s %lu", title, mp->p[mp->m-1UL]);
	for (i = mp->m-1UL; i > 0UL; i--) {
		printf(",%0*lu", P_DIGITS_MAX, mp->p[i-1UL]);
	}
	puts("");
}

int mp_inc(mp_t *mp) {
	int carry;
	unsigned long i = 0UL;
	do {
		mp->p[i]++;
		carry = mp->p[i] == P_VAL_MAX;
		if (carry) {
			mp->p[i] = 0UL;
		}
		i++;
	}
	while (i < mp->m && carry);
	if (carry) {
		unsigned long *p = realloc(mp->p, sizeof(unsigned long)*(mp->m+1UL));
		if (!p) {
			fprintf(stderr, "Error reallocating memory for mp->p\n");
			fflush(stderr);
			return 0;
		}
		mp->p = p;
		mp->p[mp->m++] = 1UL;
	}
	return 1;
}

int mp_eq_val(mp_t *mp, unsigned long val) {
	return mp->m == 1UL && mp->p[0] == val;
}

void mp_free(mp_t *mp) {
	free(mp->p);
}
