extern void test_q();
extern void test_sl();

int sl_compare(void *h1, void *h2) {
  if (h1 == h2) {
    return 0;
  }
  else if (h1 > h2) {
    return 1;
  }
  else {
    return -1;
  }
}

int main() {
  test_sl();
  return 0;
}
