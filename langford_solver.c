#include <stdio.h>
#include <stdlib.h>

#define RANGE_INF_MIN 1UL
#define ORDER_MIN 2UL
#define P_VAL_MAX 1000000000UL
#define P_DIGITS_MAX 9

typedef struct node_s node_t;

struct node_s {
	unsigned long rows_n_or_step;
	unsigned long start;
	node_t *column;
	node_t *left;
	node_t *top;
	node_t *right;
	node_t *down;
};

typedef struct {
	unsigned long m;
	unsigned long *p;
}
mp_t;

unsigned long get_group_rows(unsigned long);
unsigned long get_group_half_rows(unsigned long);
void init_column(node_t *, node_t *);
void add_group_strict_half_rows(unsigned long, unsigned long, unsigned long);
void add_group_rows(unsigned long, unsigned long, unsigned long);
void add_group_half_rows(unsigned long, unsigned long, unsigned long);
void add_group_row(unsigned long, unsigned long, unsigned long);
void init_row_node(unsigned long, unsigned long, node_t *, node_t *, node_t **);
void link_left(node_t *, node_t *);
void link_top(node_t *, node_t *);
void dlx_search(void);
unsigned long set_column_value(node_t *);
unsigned long set_row_value(node_t *);
void cover_column(node_t *);
void uncover_column(node_t *);
int mp_new(mp_t *);
void mp_print(const char *, mp_t *);
int mp_inc(mp_t *);
int mp_eq_val(mp_t *, unsigned long);
void mp_free(mp_t *);

int verbose;
unsigned long range_inf, order, sequence_size, *sequence;
node_t *nodes, **tops, *header, *row_node;
mp_t cost, solutions_n;

int main(void) {
	unsigned long range_sup, groups_n, intervals_n, columns_n, range_sup_rows_n, rows_n, i;
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
	if (scanf("%lu", &order) != 1 || order < ORDER_MIN) {
		fprintf(stderr, "Order must be greater than or equal to %lu\n", ORDER_MIN);
		fflush(stderr);
		return EXIT_FAILURE;
	}
	if (scanf("%d", &verbose) != 1) {
		fprintf(stderr, "Error reading verbose flag\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	groups_n = range_sup-range_inf+1UL;
	sequence_size = groups_n*order;
	sequence = malloc(sizeof(unsigned long)*sequence_size);
	if (!sequence) {
		fprintf(stderr, "Error allocating memory for sequence\n");
		fflush(stderr);
		return EXIT_FAILURE;
	}
	intervals_n = order-1UL;
	columns_n = sequence_size+groups_n;
	tops = malloc(sizeof(node_t *)*columns_n);
	if (!tops) {
		fprintf(stderr, "Error allocating memory for tops\n");
		fflush(stderr);
		free(sequence);
		return EXIT_FAILURE;
	}
	if (!mp_new(&cost)) {
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	if (!mp_new(&solutions_n)) {
		mp_free(&cost);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	range_sup_rows_n = get_group_rows(intervals_n*(range_sup+1UL));
	rows_n = range_sup_rows_n/2UL;
	for (i = range_sup-1UL; i >= range_inf; i--) {
		rows_n += get_group_rows(intervals_n*(i+1UL));
	}
	nodes = malloc(sizeof(node_t)*(columns_n+1UL+rows_n*(order+1UL)));
	if (!nodes) {
		fprintf(stderr, "Error allocating memory for nodes\n");
		fflush(stderr);
		mp_free(&solutions_n);
		mp_free(&cost);
		free(tops);
		free(sequence);
		return EXIT_FAILURE;
	}
	header = &nodes[columns_n];
	init_column(nodes, header);
	for (i = 0UL; i < columns_n; i++) {
		init_column(&nodes[i+1UL], &nodes[i]);
		tops[i] = &nodes[i];
	}
	row_node = &nodes[columns_n+1UL];
	add_group_strict_half_rows(range_sup+1UL, intervals_n*(range_sup+1UL), sequence_size+range_sup-range_inf);
	for (i = range_sup-1UL; i >= range_inf; i--) {
		add_group_rows(i+1UL, intervals_n*(i+1UL), sequence_size+i-range_inf);
	}
	for (i = 0UL; i < columns_n; i++) {
		link_top(&nodes[i], tops[i]);
	}
	dlx_search();
	free(nodes);
	if (range_sup_rows_n%2UL == 1UL) {
		rows_n = 1UL;
		if (range_sup > range_inf) {
			rows_n += get_group_half_rows(intervals_n*range_sup);
			for (i = range_sup-2UL; i >= range_inf; i--) {
				rows_n += get_group_rows(intervals_n*(i+1UL));
			}
		}
		nodes = malloc(sizeof(node_t)*(columns_n+1UL+rows_n*(order+1UL)));
		if (!nodes) {
			fprintf(stderr, "Error allocating memory for nodes\n");
			fflush(stderr);
			mp_free(&solutions_n);
			mp_free(&cost);
			free(tops);
			free(sequence);
			return EXIT_FAILURE;
		}
		header = &nodes[columns_n];
		init_column(nodes, header);
		for (i = 0UL; i < columns_n; i++) {
			init_column(&nodes[i+1UL], &nodes[i]);
			tops[i] = &nodes[i];
		}
		row_node = &nodes[columns_n+1UL];
		if (range_sup > range_inf) {
			add_group_row(range_sup+1UL, range_sup_rows_n/2UL, sequence_size+range_sup-range_inf);
			add_group_half_rows(range_sup, intervals_n*range_sup, sequence_size+range_sup-1UL-range_inf);
			for (i = range_sup-2UL; i >= range_inf; i--) {
				add_group_rows(i+1UL, intervals_n*(i+1UL), sequence_size+i-range_inf);
			}
		}
		for (i = 0UL; i < columns_n; i++) {
			link_top(&nodes[i], tops[i]);
		}
		dlx_search();
		free(nodes);
	}
	mp_print("Final cost", &cost);
	mp_print("Solutions", &solutions_n);
	fflush(stdout);
	mp_free(&solutions_n);
	mp_free(&cost);
	free(tops);
	free(sequence);
	return EXIT_SUCCESS;
}

unsigned long get_group_rows(unsigned long range_span) {
	return range_span < sequence_size ? sequence_size-range_span:0UL;
}

unsigned long get_group_half_rows(unsigned long range_span) {
	return range_span < sequence_size ? (sequence_size-range_span)/2UL+(sequence_size-range_span)%2UL:0UL;
}

void init_column(node_t *node, node_t *left) {
	node->rows_n_or_step = 0UL;
	link_left(node, left);
}

void add_group_strict_half_rows(unsigned long step, unsigned long range_span, unsigned long range_index) {
	unsigned long range_rows_n = get_group_rows(range_span)/2UL, i;
	for (i = range_rows_n; i > 0UL; i--) {
		add_group_row(step, i-1UL, range_index);
	}
}

void add_group_rows(unsigned long step, unsigned long range_span, unsigned long range_index) {
	unsigned long range_rows_n = get_group_rows(range_span), range_half_rows_n = range_rows_n/2UL, i;
	if (range_rows_n%2UL == 1UL) {
		add_group_row(step, range_half_rows_n, range_index);
		for (i = 1UL; i <= range_half_rows_n; i++) {
			add_group_row(step, range_half_rows_n-i, range_index);
			add_group_row(step, range_half_rows_n+i, range_index);
		}
	}
	else {
		for (i = 1UL; i <= range_half_rows_n; i++) {
			add_group_row(step, range_half_rows_n-i, range_index);
			add_group_row(step, range_half_rows_n+i-1UL, range_index);
		}
	}
}

void add_group_half_rows(unsigned long step, unsigned long range_span, unsigned long range_index) {
	unsigned long range_rows_n = get_group_half_rows(range_span), i;
	for (i = range_rows_n; i > 0UL; i--) {
		add_group_row(step, i-1UL, range_index);
	}
}

void add_group_row(unsigned long step, unsigned long start, unsigned long range_index) {
	unsigned long i;
	init_row_node(step, start, &nodes[start], row_node+order, &tops[start]);
	for (i = 1UL; i < order; i++) {
		init_row_node(step, start, &nodes[start+i*step], row_node-1, &tops[start+i*step]);
	}
	init_row_node(step, start, &nodes[range_index], row_node-1, &tops[range_index]);
}

void init_row_node(unsigned long step, unsigned long start, node_t *column, node_t *left, node_t **top) {
	row_node->rows_n_or_step = step;
	row_node->start = start;
	row_node->column = column;
	column->rows_n_or_step++;
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
	top->down = node;
}

void dlx_search(void) {
	unsigned long value_max, i;
	node_t *column_min, *column, *row;
	if (!mp_inc(&cost)) {
		return;
	}
	if (header->right == header) {
		if (!mp_inc(&solutions_n)) {
			return;
		}
		if (mp_eq_val(&solutions_n, 1UL) || verbose) {
			mp_print("Cost", &cost);
			printf("%lu", sequence[0]);
			for (i = 1UL; i < sequence_size; i++) {
				printf(" %lu", sequence[i]);
			}
			puts("");
			fflush(stdout);
		}
		return;
	}
	column_min = header->right;
	value_max = set_column_value(column_min);
	for (column = column_min->right; column != header; column = column->right) {
		if (column->rows_n_or_step < column_min->rows_n_or_step) {
			if (column->rows_n_or_step == 0UL) {
				return;
			}
			column_min = column;
			value_max = set_column_value(column_min);
		}
		else if (column->rows_n_or_step == column_min->rows_n_or_step) {
			unsigned long value = set_column_value(column);
			if (value > value_max) {
				column_min = column;
				value_max = value;
			}
		}
	}
	cover_column(column_min);
	for (row = column_min->down; row != column_min; row = row->down) {
		node_t *node;
		for (i = 0UL; i < order; i++) {
			sequence[row->start+i*row->rows_n_or_step] = row->rows_n_or_step-1UL;
		}
		for (node = row->right; node != row; node = node->right) {
			cover_column(node->column);
		}
		dlx_search();
		for (node = row->left; node != row; node = node->left) {
			uncover_column(node->column);
		}
	}
	uncover_column(column_min);
}

unsigned long set_column_value(node_t *column) {
	unsigned long value = 0UL;
	node_t *row;
	for (row = column->down; row != column; row = row->down) {
		value += set_row_value(row);
	}
	return value;
}

unsigned long set_row_value(node_t *row) {
	unsigned long value = 0UL;
	node_t *node;
	for (node = row->right; node != row; node = node->right) {
		value += node->column->rows_n_or_step;
	}
	return value;
}

void cover_column(node_t *column) {
	node_t *row;
	column->right->left = column->left;
	column->left->right = column->right;
	for (row = column->down; row != column; row = row->down) {
		node_t *node;
		for (node = row->right; node != row; node = node->right) {
			node->column->rows_n_or_step--;
			node->down->top = node->top;
			node->top->down = node->down;
		}
	}
}

void uncover_column(node_t *column) {
	node_t *row;
	for (row = column->top; row != column; row = row->top) {
		node_t *node;
		for (node = row->left; node != row; node = node->left) {
			node->top->down = node;
			node->down->top = node;
			node->column->rows_n_or_step++;
		}
	}
	column->left->right = column;
	column->right->left = column;
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
