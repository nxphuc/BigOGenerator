# Generator tool hỗ trợ tạo tests cho Big-O coding.

**WARNING!!!**: tool chỉ hỗ trợ tạo test nhanh hơn, không hỗ trợ tạo test chặt hơn, muốn tạo test chặt thì phải phân chia trường hợp và lên chiến lượt cho từng trường hợp trước khi tạo test.

Thư viện hỗ trợ random các đối tượng trong quá trình tạo test case các bài toán thuật toán. Sử dụng `random.h` như một thư viện bình thường. Cần biên dịch với chuẩn C++11 (`-std=c++11`) hoặc hơn.

Có 3 thành phần chính trong thư viện:

## 1. Class Random

Chứa toàn bộ các hàm random ngẫu nhiên thường dùng. Được xây dựng dựa trên ý tưởng của [testlib](https://github.com/MikeMirzayanov/testlib), nhưng được xây dựng lại bằng thuật toán Xor Shift 128 được sử dụng trong V8 JavaScript engine và bổ sung một số chức năng mà mình cảm thấy testlib chưa hỗ trợ

| Hàm | Chức năng |
|--------|---------|
| _Random()_ | Constructor, tạo generator với seed dựa theo giá trị hiện tại của [time_since_epoch](https://cplusplus.com/reference/chrono/time_point/time_since_epoch/) |
|Random(seed)| Constructor, tạo generator với `seed` cho trước |
|int64_t initial_seed| Lấy giá trị `seed` hiện tại |
|SetSeed(int64_t seed) | Thay đổi `seed` của generator |
| int32_t Next(int32_t max) | Random một số nguyên 32 bit nằm trong khoảng [`0`, `max`) |
| int32_t Next(int32_t min, int32_t max) | Random một số nguyên 32 bit trong khoảng [`min`, `max`] |
| int64_t Next(int64_t max) | Random một số nguyên 64 bit nằm trong khoảng [`0`, `max`) |
| int64_t Next(int64_t min, int64_t max) | Random một số nguyên 6 bit trong khoảng [`min`, `max`] |
| double Next() | Random một số thực trong khoảng [`0`, `1`) |
| double Next(double max) | Random một số thực trong khoảng [`0`, `max`) |
| double Next(double min, double max) | Random một số thực trong khoảng [`0`, `max`) |
| value_type Any(const Container<T\> &c)| Trả về một phần tử ngẫu nhiên trong container `c` |
| value_type Any(const Iter &first, const Iter &last) | Trả về một phần tử ngẫu nhiên trong tập hợp được quản lý bởi 2 Iterator `first` và `last` |
| std::vector<T> Permutation(T size) | Tạo ra hoán vị của các số nguyên trong đoạn [`0`, `size - 1`] |
| std::vector<E> Permutation(T size, E first) | Trả về hoán vị của `size` số trong đoạn [`first`, `first + size - 1`] |
| std::vector<T> NextSet(int32_t size, T max) | Trả về một tập gồm `size` phần tử phân biệt trong khoảng [`0`, `max`) |
| std::vector<T> NextSet(int32_t size, T min, T max) | Trả về một tập gồm `size` phần tử phân biệt trong đoạn [`min`, `max`] |
| std::string NextString(const std::string& pattern) | Trả về một chuỗi ngẫu nhiên theo pattern p <br> Pattern là một mẫu regex đơn giản |
| void Shuffle(RandomAccessIterator first, RandomAccessIterator last) | shuffle ngẫu nhiên các phần tử trong tập hợp thuộc khoảng iterator [`first`, `last`) |
| std::vector<std::pair<int32_t, int32_t\>\> GenerateRootedTree(size, first = 0) | Random ngẫu nhiên và trả về danh sách cạnh của cây `size` đỉnh, các đỉnh đánh số từ `first` |
| std::vector<int32_t\> GenerateRootedTree(size, root = 0) | Random ngẫu nhiên và trả về mảng `parent` với `parent[i]` là cha của đỉnh i trong đồ thị gồm `size` đỉnh, có gốc là `root` |

## 2. Namespace `printer`

Chứa các hàm hỗ trợ in nhanh các đối tượng. Toàn bộ đều sẽ in xuống luồng xuất chuẩn (`stdout`)

| Hàm | Chức năng |
|--------|---------|
| Print(...) | In một hoặc nhiều đối tượng trên cùng 1 dòng, phân tách bằng khoảng trắng.<br>Nếu là các đối tượng đơn khác nhau, cho phép giới hạn in 5 đố tượng 1 lúc.<br> Nếu là kiểu tập hợp (`array`, `vector`, `set`, `map`, ...), sẽ in các phần tử trong tập hợp trên cùng 1 dòng |
| PrintLine(...) | Tương tự như `Print()`, nhưng xuống dòng sau khi in xong các phần tử |

## 3. Các hàm hỗ trợ chung

- _std::string `Format`(const char *`format`, ...`args`)_: trả về một chuỗi sau khi format. Ví dụ `Format("%d + %d = %d", 1, 2, 1+2)` sẽ trả về string "1 + 2 = 3"
- _void `OpenTestFile`(int `test`, std::string `ext`)_: Mở file một file có tên `${test}.{ext}` (`ext` mặc định là `in`) và redirect vào `stdout` để ghi test case.


## Bugs

- Chưa sử dụng được các hàm `Print`/`PrintLine` với mảng char[]
- Chưa sử dụng được các hàm `Print`/`PrintLine` với Containter chứa `std::pair` (vẫn sử dụng được để in `std::pair`)