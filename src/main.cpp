#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

void work(const int64_t &r, const int64_t tries, std::atomic<int64_t> &hits_count, int64_t seed)
{
  std::mt19937                           gen(seed);
  std::uniform_real_distribution<double> dist(0.0, r);

  int64_t hits_count_local = 0;
  for (int64_t i = 0; i < tries; ++i)
  {
    double x = dist(gen);
    double y = dist(gen);
    hits_count_local += ((x * x + y * y) <= r * r);
  }

  hits_count.fetch_add(hits_count_local, std::memory_order_relaxed);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "Call " << argv[0] << " <tries> <gen_init>\n";
    return 1;
  }
  else if (std::stoi(argv[1]) < 0)
  {
    std::cerr << "Call " << argv[0] << " <tries> <gen_init> with tries > 0\n";
    return 1;
  }
  else if (argc == 3 && std::stoi(argv[2]) < 0)
  {
    std::cerr << "Call " << argv[0] << " <tries> <gen_init> with gen_init > 0\n";
    return 1;
  }

  int64_t tries    = std::stoll(argv[1]);
  int64_t gen_init = argc == 3 ? std::stoll(argv[2]) : 0;

  int64_t n_threads = 0;
  int64_t r         = 0;
  while (std::cin >> r >> n_threads)
  {

    if (r < 0 || n_threads < 0)
    {
      std::cerr << "Radius of circle and number of threads must be positive\n";
      return 1;
    }

    // Точки, попавшие в окружность
    std::atomic<int64_t>     hits_count = 0;
    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    int64_t base      = tries / n_threads;
    int64_t remainder = tries % n_threads;
    // Стартуем отсчет времени
    auto start = std::chrono::high_resolution_clock::now();

    for (int64_t i = 0; i < n_threads; ++i)
    {
      threads.emplace_back(work, std::cref(r), base + (!i ? remainder : 0), std::ref(hits_count), gen_init);
    }
    // Дожидаемся потоки
    for (auto &t : threads)
    {
      t.join();
    }
    auto    end         = std::chrono::high_resolution_clock::now();
    auto    duration    = end - start;
    auto    ms          = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    int64_t square_area = r * r;
    // 4 - так как в функции work мы пуляли точками только в первый квадрант [0,r] x [0,r]
    double area = 4 * square_area * (static_cast<double>(hits_count) / tries);

    std::cout << std::setprecision(4) << ms.count() << " " << area << "\n";
  }
  return 0;
}