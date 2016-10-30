#include "box.h"

int bounding_box_contains(const BoundingBox *box, const int x, const int y) {
  if (x >= box->min_x && x <= box->max_x) {
    return y >= box->min_y && y <= box->max_y;
  }
  return 0;
}

/**
 * Compares two BoundingBox objects for equality.
 */
int bounding_box_equals(const BoundingBox *a, const BoundingBox *const b) {
  const int min_equals = a->min_x == b->min_x && a->min_y == b->min_y;
  const int max_equals = a->max_x == b->max_x && a->max_y == b->max_y;
  return min_equals && max_equals;
}

long bounding_box_area(const BoundingBox *box) {
  const long w = box->max_x - box->min_x + 1;
  const long h = box->max_y - box->min_y + 1;
  return w * h;
}
