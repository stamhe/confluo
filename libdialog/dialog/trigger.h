#ifndef DIALOG_TRIGGER_H_
#define DIALOG_TRIGGER_H_

#include "aggregate.h"
#include "schema_snapshot.h"

namespace dialog {
namespace monitor {
struct trigger {
 public:
  trigger(const std::string& trigger_name, const std::string& filter_name,
          const std::string& trigger_expr, aggregate_id agg,
          const std::string& field_name, size_t field_idx,
          const data_type& field_type, relop_id op, const numeric& threshold)
      : trigger_name_(trigger_name),
        filter_name_(filter_name),
        trigger_expr_(trigger_expr),
        agg_id_(agg),
        field_name_(field_name),
        field_idx_(field_idx),
        field_type_(field_type),
        op_(op),
        threshold_(threshold),
        is_valid_(true) {
  }

  aggregate create_aggregate() {
    return
        agg_id_ == aggregate_id::D_CNT ?
            aggregate(LONG_TYPE, agg_id_) : aggregate(field_type_, agg_id_);
  }

  std::string trigger_name() const {
    return trigger_name_;
  }

  std::string filter_name() const {
    return filter_name_;
  }

  std::string trigger_expr() const {
    return trigger_expr_;
  }

  aggregate_id agg_id() const {
    return agg_id_;
  }

  size_t field_idx() const {
    return field_idx_;
  }

  data_type field_type() const {
    return field_type_;
  }

  relop_id op() const {
    return op_;
  }

  const numeric threshold() const {
    return threshold_;
  }

  numeric zero() {
    return aggregators[agg_id_].zero(field_type_);
  }

  numeric agg(const numeric& a, const schema_snapshot& s, void* data) {
    numeric b(s.get(data, field_idx_));
    return aggregators[agg_id_].agg(
        a.type().id == type_id::D_NONE ? zero() : a,
        agg_id_ == aggregate_id::D_CNT ? count_one : b);
  }

  bool invalidate() {
    bool expected = true;
    if (atomic::strong::cas(&is_valid_, &expected, false)) {
      return true;
    }
    return false;
  }

  bool is_valid() const {
    return atomic::load(&is_valid_);
  }

  std::string to_string() const {
    return "Trigger Name: " + trigger_name_ + " Filter Name: " + filter_name_
        + " Trigger Expression: " + aggop_utils::agg_to_string(agg_id_) + "("
        + field_name_ + ") " + relop_utils::op_to_str(op_) + " "
        + threshold_.to_string();
  }

 private:
  std::string trigger_name_;
  std::string filter_name_;
  std::string trigger_expr_;
  aggregate_id agg_id_;
  std::string field_name_;
  uint32_t field_idx_;
  data_type field_type_;
  relop_id op_;
  numeric threshold_;
  atomic::type<bool> is_valid_;
};
}
}

#endif /* DIALOG_TRIGGER_H_ */