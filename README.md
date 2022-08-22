# Generator tool hỗ trợ tạo tests cho Big-O coding.

**WARNING!!!**: tool chỉ hỗ trợ tạo test nhanh hơn, không hỗ trợ tạo test chặt hơn, muốn tạo test chặt thì phải phân chia trường hợp và lên chiến lượt cho từng trường hợp trước khi tạo test.

Thư viện hỗ trợ random các đối tượng trong quá trình tạo test case các bài toán thuật toán. Sử dụng `random.h` như một thư viện bình thường. Cần biên dịch với chuẩn C++11 (`-std=c++11`) hoặc mới hơn.

## Cách sử dụng:

Chỉ cần copy file `random.h` vào folder chứa code tạo test của bạn và import file vào để sử dụng.

## Các thành phần chính:

Có 3 thành phần chính trong thư viện:

### 1. Class Random

Chứa toàn bộ các hàm random ngẫu nhiên thường dùng. Được xây dựng dựa trên ý tưởng của [testlib](https://github.com/MikeMirzayanov/testlib), nhưng được xây dựng lại bằng thuật toán Xor Shift 128 được sử dụng trong V8 JavaScript engine và bổ sung một số chức năng mà mình cảm thấy testlib chưa hỗ trợ

| Hàm | Chức năng |
|--------|---------|
| _Random()_ | Constructor, tạo generator với seed dựa theo giá trị hiện tại của [time_since_epoch](https://cplusplus.com/reference/chrono/time_point/time_since_epoch/) |
| Random(uint64_t seed) | Constructor, tạo generator với `seed` cho trước |
| Random(const char* str) | Constructor, tạo generator với `seed` được tính theo chuỗi `str` |
| Random(const std::string& str) | Constructor, tạo generator với `seed` được tính theo chuỗi `str` |
| Random(const int32_t agrc, const char* argv[]) | Constructor, tạo generator với `seed` được tính theo mảng `argv` gồm `argc` phần tử. Có thể sử dụng constructor này để tạo `rnd` khi chạy code với command line |
| int64_t initial_seed | Lấy giá trị `seed` hiện tại |
| SetSeed(int64_t seed) | Thay đổi `seed` của generator |
| int32_t Next(int32_t max) | Random một số nguyên 32 bit nằm trong khoảng [`0`, `max`) |
| int32_t Next(int32_t min, int32_t max) | Random một số nguyên 32 bit trong khoảng [`min`, `max`] |
| int64_t Next(int64_t max) | Random một số nguyên 64 bit nằm trong khoảng [`0`, `max`) |
| int64_t Next(int64_t min, int64_t max) | Random một số nguyên 6 bit trong khoảng [`min`, `max`] |
| double Next() | Random một số thực trong khoảng [`0`, `1`) |
| double Next(double max) | Random một số thực trong khoảng [`0`, `max`) |
| double Next(double min, double max) | Random một số thực trong khoảng [`0`, `max`) |
| T Any(const Container<T\> &c)| Trả về một phần tử ngẫu nhiên trong container `c` |
| T Any(const Iter &first, const Iter &last) | Trả về một phần tử ngẫu nhiên trong tập hợp được quản lý bởi 2 Iterator `first` và `last` |
| std::vector<T> Permutation(T size) | Tạo ra hoán vị của các số nguyên trong đoạn [`0`, `size - 1`] |
| std::vector<E> Permutation(T size, E first) | Trả về hoán vị của `size` số trong đoạn [`first`, `first + size - 1`] |
| std::vector<T> NextSet(int32_t size, T max) | Trả về một tập gồm `size` phần tử phân biệt trong khoảng [`0`, `max`) |
| std::vector<T> NextSet(int32_t size, T min, T max) | Trả về một tập gồm `size` phần tử phân biệt trong đoạn [`min`, `max`] |
| std::string NextString(const std::string& pattern) | Trả về một chuỗi ngẫu nhiên theo pattern p <br> Pattern là một mẫu regex đơn giản |
| void Shuffle(RandomAccessIterator first, RandomAccessIterator last) | shuffle ngẫu nhiên các phần tử trong tập hợp thuộc khoảng iterator [`first`, `last`) |
| std::vector<std::pair<int32_t, int32_t\>\> GenerateRootedTree(size, first = 0, weight = 0) | Random ngẫu nhiên và trả về danh sách cạnh của cây `size` đỉnh, các đỉnh đánh số từ `first`, có trọng số hàm random là `weight` (xem phần `WeightedNext(...)`). Về mặt tương đối:<br><ul><li>`weight` càng âm thì cây càng thấp (các đỉnh nối với các đỉnh gần gốc hơn)</li><li>`weight` càng dương thì cây càng cao</li></ul> |
| std::vector<int32_t\> GenerateRootedTree(size, root = 0) | Random ngẫu nhiên và trả về mảng `parent` với `parent[i]` là cha của đỉnh i trong đồ thị gồm `size` đỉnh, có gốc là `root` |
| T WeightedNext(...) | Chức năng tương tự như Next, nhưng có thêm tha số `weight` <br><ul><li>weight = 0: vai trò như `Random::Next()`</li><li>weight > 0: trả về maximum của `weight+1` lần `Next(...)`</li><li>weight < 0: trả về minimum của `weight+1` lần `Next(...)`</li></ul> |
| T WeightedAny(...) | Tương tự WeightedNext() |


### 2. Namespace `printer`

Chứa các hàm hỗ trợ in nhanh các đối tượng. Toàn bộ đều sẽ in xuống luồng xuất chuẩn (`stdout`)

| Hàm | Chức năng |
|--------|---------|
| Print(...) | In một hoặc nhiều đối tượng trên cùng 1 dòng, phân tách bằng khoảng trắng.<br>Nếu là các đối tượng đơn khác nhau, cho phép giới hạn in 5 đố tượng 1 lúc.<br> Nếu là kiểu tập hợp (`vector`, `set`, `map`, ...), sẽ in các phần tử trong tập hợp trên cùng 1 dòng |
| PrintLine(...) | Tương tự như `Print()`, nhưng xuống dòng sau khi in xong các phần tử |

Lưu ý: Đối với mảng tĩnh, có thể in bằng 2 cách: `Print/PrintLine(arr, size)` hoặc `Print/PrintLine(arr, arr + size)`.

**Ví dụ**
```cpp
int a[] = {1, 2, 3, 4, 5};
printer::PrintLine(a, 5);
printer::PrintLine(1, '2', 3.0, "this is a string");
printer::PrintLine("another string");
```

Hoặc

```cpp
using namespace printer;
int a[] = {1, 2, 3, 4, 5};
PrintLine(a, 5);
PrintLine(1, '2', 3.0, "this is a string");
PrintLine("another string");
```

Kết quả:

```console
1 2 3 4 5
1 2 3 this is a string
another string
```

### 3. Các hàm hỗ trợ chung

- _`std::string Format(const char *format, ...args)`_: trả về một chuỗi sau khi format, tương tự như hàm `printf`. Ví dụ `Format("%d + %d = %d", 1, 2, 1+2)` sẽ trả về "`1 + 2 = 3`"
- _`void OpenTestFile(int test_number, std::string ext)`_: Mở file một file có tên `${test_number}.{ext}` (`ext` mặc định là `in`) và redirect vào `stdout` để ghi test case.

## Mã nguồn minh họa:

```cpp
#include <bits/stdc++.h>
#include "random.h"

int main(int argc, char* argv[]) {
    Random rnd;
    int cnt[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::cout << rnd.Next() << "\n";   // random số thực trong khoảng [0.0, 1.0)
    std::cout << rnd.Next(10) << "\n"; // random số nguyên trong khoảng [0, 10)
    std::cout << rnd.Next(10LL, 100LL) << "\n"; // random số nguyên 64-bits trong khoảng [10, 100]
    
    int a[] = {23, 12, 45, 98, 0, 5, 33, 59, 75, 77};
    std::cout << rnd.Any(a + 0, a + 10) << "\n";
    rnd.Shuffle(a, a+10);
    printer::PrintLine(a, 10);
    
    std::vector<int> perm = rnd.Permutation(10);
    printer::PrintLine(perm);

    return 0;
}
```

## Bugs

- Chưa sử dụng được các hàm `Print`/`PrintLine` với Containter chứa `std::pair` (vẫn sử dụng được để in `std::pair`)