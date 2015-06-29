#include <algorithm>
#include <cassert>
#include <vector>

namespace dlx {

class DLX {
protected:
  struct Node {
    Node *left, *right, *up, *down;
    Node* column;
    union {
      size_t size; // column header
      size_t row; // normal node
    };
  };
  typedef Node* NodeBlock;

  NodeBlock NewBlock(size_t count) {
    node_blocks_.push_back(new Node[count]);
    return node_blocks_.back();
  }

  void CoverColumn(Node* column) {
    column->right->left = column->left;
    column->left->right = column->right;
    for (Node* r = column->down; r != column; r = r->down) {
      for (Node* c = r->right; c != r; c = c->right) {
        c->down->up = c->up;
        c->up->down = c->down;
        c->column->size--;
      }
    }
  }

  void UncoverColumn(Node* column) {
    for (Node* r = column->up; r != column; r = r->up) {
      for (Node* c = r->left; c != r; c = c->left) {
        c->down->up = c;
        c->up->down = c;
        c->column->size++;
      }
    }
    column->right->left = column;
    column->left->right = column;
  }

  bool Search() {
    if (head_->right == head_) {
      return true;
    }
    Node* min_col = nullptr;
    for (Node* c = head_->right; c->right != head_; c = c->right) {
      if (min_col == nullptr || c->size < min_col->size) {
        min_col = c;
        if (min_col->size == 1) break;
      }
    }
    CoverColumn(min_col);
    for (Node* r = min_col->down; r != min_col; r = r->down) {
      // try using row r
      selected_rows_.push_back(r->row);
      for (Node* c = r->right; c != r; c = c->right) CoverColumn(c->column);
      if (Search()) return true;
      for (Node* c = r->left; c != r; c = c->left) UncoverColumn(c->column);
      selected_rows_.pop_back();
    }
    UncoverColumn(min_col);
    return false;
  }

public:
  DLX(size_t n_cols):
      covered_(n_cols, false), n_cols_(n_cols), n_rows_(0), head_(nullptr) {
    NodeBlock nodes = NewBlock(n_cols + 1);
    head_ = nodes + n_cols;
    for (size_t i = 0; i < n_cols + 1; i++) {
      nodes[i].right = &nodes[(i + 1) % (n_cols + 1)];
      nodes[(i + 1) % (n_cols + 1)].left = &nodes[i];
      nodes[i].up = nodes[i].down = &nodes[i];
      nodes[i].column = &nodes[i];
      nodes[i].size = 0;
    }
  };
  ~DLX() {
    for (auto block : node_blocks_) delete [] block;
  }

  void AddRow(std::vector<size_t> indices) {
    NodeBlock nodes = NewBlock(indices.size());
    NodeBlock columns = node_blocks_[0];
    std::sort(indices.begin(), indices.end());
    size_t n = std::unique(indices.begin(), indices.end()) - indices.begin();
    for (size_t i = 0; i < n; i++) {
      nodes[i].right = &nodes[(i + 1) % n];
      nodes[(i + 1) % n].left = &nodes[i];
      Node* column = columns + indices[i];
      nodes[i].up = column->up;
      nodes[i].down = column;
      nodes[i].row = n_rows_;
      nodes[i].column = column;
      column->up = column->up->down = nodes + i;
      column->size++;
    }
    n_rows_++;
  }

  void PreCoverColumn(size_t index) {
    covered_[index] = true;
  }

  bool Solve() {
    NodeBlock columns = node_blocks_[0];
    for (size_t i = 0; i < n_cols_; i++) if (covered_[i]) {
      CoverColumn(&columns[i]);
    }
    for (Node* c = head_->right; c != head_; c = c->right) {
      assert(c->size > 0);
    }
    return Search();
  }

  std::vector<size_t> GetSolution() {
    return selected_rows_;
  }

private:
  size_t n_cols_, n_rows_;
  std::vector<bool> covered_;
  std::vector<NodeBlock> node_blocks_;
  std::vector<size_t> selected_rows_;
  Node* head_;
};
}
