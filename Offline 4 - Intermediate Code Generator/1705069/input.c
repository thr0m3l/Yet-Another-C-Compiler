int sum(int n) {
  if (n != 0)
    return n + sum(n - 1);
  else
    return n;
}

int rec_factorial(int n) {
  if (n > 0)
    return n * rec_factorial(n - 1);
  else
    return 1;
}

int fib(int n) {
  int a, b;
  if (n <= 1)
    return n;
  return fib(n - 1) + fib(n - 2);
}

int itr_factorial(int n) {
  int arr[10], i;
  arr[0] = 1;
  arr[1] = 1;
  for (i = 2; i <= n; i++) {
    arr[i] = arr[i - 1] * i;
  }
  return arr[n];
}

int main() {
  int x;
  x = rec_factorial(7);
  println(x);
}
