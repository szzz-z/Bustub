#include <memory>
#include <vector>
#include "execution/expressions/abstract_expression.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/expressions/comparison_expression.h"
#include "execution/expressions/logic_expression.h"
#include "execution/plans/abstract_plan.h"
#include "execution/plans/hash_join_plan.h"
#include "execution/plans/nested_loop_join_plan.h"
#include "optimizer/optimizer.h"

namespace bustub {

auto ParseExpr(const AbstractExpressionRef &pre, std::vector<ComparisonExpression *> &cmp_exprs) -> bool {
  auto cmp = dynamic_cast<ComparisonExpression *>(pre.get());
  if (cmp != nullptr) {
    if (cmp->comp_type_ != ComparisonType::Equal) {
      return false;
    }
    cmp_exprs.push_back(cmp);
    return true;
  }
  auto logic = dynamic_cast<LogicExpression *>(pre.get());
  if (logic != nullptr) {
    if (logic->logic_type_ != LogicType::And) {
      return false;
    }
    for (const auto &child : logic->children_) {
      if (!ParseExpr(child, cmp_exprs)) {
        return false;
      }
    }

    return true;
  }

  return false;
}

auto Optimizer::OptimizeNLJAsHashJoin(const AbstractPlanNodeRef &plan) -> AbstractPlanNodeRef {
  // TODO(student): implement NestedLoopJoin -> HashJoin optimizer rule
  // Note for 2023 Spring: You should at least support join keys of the form:
  // 1. <column expr> = <column expr>
  // 2. <column expr> = <column expr> AND <column expr> = <column expr>
  std::vector<AbstractPlanNodeRef> children;
  for (const auto &child : plan->GetChildren()) {
    children.emplace_back(OptimizeNLJAsHashJoin(child));
  }
  auto optimized_plan = plan->CloneWithChildren(std::move(children));

  if (optimized_plan->GetType() != PlanType::NestedLoopJoin) {
    return optimized_plan;
  }

  const auto &nlj_plan = dynamic_cast<const NestedLoopJoinPlanNode &>(*optimized_plan);

  std::vector<ComparisonExpression *> cmp_exprs;
  if (!ParseExpr(nlj_plan.Predicate(), cmp_exprs)) {
    return optimized_plan;
  }

  std::vector<AbstractExpressionRef> left_key_expr;
  std::vector<AbstractExpressionRef> right_key_expr;
  left_key_expr.reserve(cmp_exprs.size());
  right_key_expr.reserve(cmp_exprs.size());
  for (const auto &cmp : cmp_exprs) {
    auto *left_expr = dynamic_cast<ColumnValueExpression *>(cmp->children_[0].get());
    auto *right_expr = dynamic_cast<ColumnValueExpression *>(cmp->children_[1].get());
    if (left_expr == nullptr || right_expr == nullptr) {
      return optimized_plan;
    }
    if (left_expr->GetTupleIdx() == 0) {
      left_key_expr.emplace_back(
          std::make_shared<ColumnValueExpression>(0, left_expr->GetColIdx(), left_expr->GetReturnType()));
      right_key_expr.emplace_back(
          std::make_shared<ColumnValueExpression>(1, right_expr->GetColIdx(), right_expr->GetReturnType()));
    } else if (left_expr->GetTupleIdx() == 1) {
      left_key_expr.emplace_back(
          std::make_shared<ColumnValueExpression>(0, right_expr->GetColIdx(), right_expr->GetReturnType()));
      right_key_expr.emplace_back(
          std::make_shared<ColumnValueExpression>(1, left_expr->GetColIdx(), left_expr->GetReturnType()));
    }
  }

  return std::make_shared<HashJoinPlanNode>(nlj_plan.output_schema_, nlj_plan.GetLeftPlan(), nlj_plan.GetRightPlan(),
                                            std::move(left_key_expr), std::move(right_key_expr),
                                            nlj_plan.GetJoinType());
}

}  // namespace bustub
