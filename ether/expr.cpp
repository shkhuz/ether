#include <expr.hpp>
#include <token.hpp>

u64 get_expr_char_count(Expr* expr) {
	return expr->tail->end - expr->head->start;
}
