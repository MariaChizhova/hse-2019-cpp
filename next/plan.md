# Долги
Шаблонные друзья: https://blog.panicsoftware.com/friends-and-where-to-find-them/
thread_local переменные

friend class

Пример для `auto foo() ->`: возвращаемое значение — приватный тип для структуры. Смотри технофлуд от 20.04.2020 12:50 MSK.
Обязательно читать ошибки сверху вниз.

* User defined literals, не на экзамен: https://en.cppreference.com/w/cpp/language/user_literal
  * Пример из chrono: `auto duration = 10s` (секунд).
  * Пример для автовывода типов: `split(....) == vector{"foo"sv}` (иначе был бы `vector<char*>`).
  * Можно писать свои, можно даже парсить длинные числа руками.
* Был `dynamic_cast`. А для `shared_ptr` есть аналогичный `dynamic_pointer_cast` (и ещё три аналогичных `*_pointer_cast`).
  Это популярно для умных указателей, если вообще имеет смысл менять тип указателя, не меняя тип владения
  (для `unique_ptr` не имеет).
* CRTP:
  * https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Curiously_Recurring_Template_Pattern
  * Можно реализовать для реализации `struct Point : operators<Point> { bool operator<(..); }`
  * Можно для вынесения любой другой функциональности в общего предка без виртуальных функций.
* Pimpl для выноса приватных полей и методов из заголовка и сохранения API/ABI. Ценой динамических выделений памяти.

#
## SFINAE [00:05]
Цель: научиться определять, есть ли у типа определённый метод вроде `iterator begin()`.

http://jguegant.github.io/blogs/tech/sfinae-introduction.html

Базовый блок: Substitution Failure Is Not An Error (SFINAE).

* Если в _объявлении_ функции случилась ошибка при подстановке шаблонных типов, забиваем на эту перегрузку.
* Пример: `template<typename T> typename iterator_traits<T>::value_type dereference(T it) { return *it; }`
* Пример с деталями:
  * В процессе подстановки default template parameter тоже окей
  * Default values уже не часть объявления для целей SFINAE, будет hard ошибка.
  * ЗАБИТЬ: почему `typename = enable_if` не работает, а `enable_if_t<>* = nullptr` работает: https://stackoverflow.com/a/31138725/767632

## Member detector idiom, C++98 style [00:10]
TODO

Добавляем две перегрузки функции `detector`: одна возвращает `yes`, другая `no`, причём первая вырезается по SFINAE

* Вторая принимает ellipsis и выберется последней. Ей тоже нужны шаблонные параметры.
* У первой функции надо подобрать такие параметры и шаблонные типы, чтобы выводились только при наличии `begin()`.
  * Можно сказать, что это будет какое-то количество указателей неважно куда и мы вызовем с `nullptr`.
  * Тогда результат выражения — либо `yes`, либо `no`, и по `sizeof()` мы можем определить, что именно.

* Попросим в параметре указатель на член определённого типа и скажем, что он равен `&C::begin`
  * Увы, в параметрах по умолчанию SFINAE не работает.
* Костылим вроде `integer_sequence`: делаем свой `template<typename T, T> struct Check{}`
  и просим в качестве параметра указатель на `Check<typename C::iterator (C::*)(), &C::begin>`.

* Теперь по `sizeof(detector<T>(nullptr))` можем определить, 

##
declval + comma operator in return value

# Метапрограммирование
1. clang + enable_if = хорошие сообщения об ошибках. проверить, показать
2. если мы делаем свой sfinae detector, то осторожно с ADL при его вызове.
3. enable_if + шаблонный параметр класса, а не метода: надо вводить фиктивные параметры.
4. шаблонные параметры конструктора не указать явно?
5. SFINAE не детектит ошибки инстанцирования. Сразу проблемы с is_default_constructible<pair<>>, если pair реализован с наивным конструктором по умолчанию без SFINAE.
6. Нельзя шаблонизировать конструкторы/операторы копирования/присваивания. НО КАК РАБОТАЕТ SWAP TRICK. ЧИТАТЬ СТАНДАРТ. Видимо, надо ооочень аккуратно смотреть на user-defined/user-declared/implicitly defined-deleted, whatever.
7. https://habr.com/ru/company/jugru/blog/467299/
    подключение special memberes через наследование.
8. std::ref/std::cref/std::reference_wrapper, чтобы показать, как передавать какие параметры. Но зачем? Чтобы везде по значению?
   Чтобы работал автовывод типов при вызове. Например, `int x; thread_pool.enqueue(f, x);` — тут брать `x` по ссылке или копировать? rvalue, понятно, копировать.
   Поэтому давайте копировать всё сразу. А если что-то надо именно захватить, то `ref`/`cref`.
   Важная стандартная тонкость!

# Правила вывода типов [00:20]
## Где случается [00:07]
CppCon 2014: Scott Meyers "Type Deduction and Why You Care"
(первые полчаса; забить на `decltype`, `T&&`/universal references)

В C++03 был только автовывод параметров у функций, просто работало.

* Дано: `template<typename T> void foo(ТИП x);` и вызываем `foo(value);`.
  Надо найти: `T = ?`, `ТИП = ?`.
  Они могут отличаться.
  Например, если `ТИП=vector<T>` или `ТИП = T&` или `ТИП=vector<T*>*`.
* В C++11 появляются `auto` переменные:
  ```
  auto x = max(10, 20);
  map<int, int> my_map;
  auto it = my_map.begin();
  ```
  Здесь слово `auto` играет роль `T`.
* Также в C++11 появляются лямбды, у них надо возвращаемое значение:
  ```
  auto lambda = [&](int a, int b) /* -> bool*/ { return a * x < b * x; }
  ```
* А в C++14 появляется вывод типов для любых функций: вывели для первого `return`.
  Дальше они все должны совпасть.
  Если `return`'ов нет, то `void`.
  ```
  auto sum(int n) {
      if (n == 0) return 0;
      else return n + sum(n - 1);  // Окей, тип уже знаем. Поменять местами не окей.
  }
  ```
* Ещё в C++14 появляются lambda init capture:
  ```
  int x = 10, y = 20;
  auto lambda = [z = x + y](int a) { return a * z; };
  assert(lambda(-1) == -30);
  ```

## Общая схема [00:02]
Идём рекурсивно по `ТИП` сверху вниз.
Будем разбирать такой пример:
```
template<typename T> void foo(T &x); // ТИП=T&
int x;
const int cx = x;
const int &rx = x;
```

## По значению [00:05]
Если видим `НЕЧТО` без ссылок и констант: хотим копию значения.

* Если `value` — ссылка, убери это.
* Если `value` — `const` или `volatile` на верхнем уровне, убери это.
* То, что осталось — делай pattern matching с `НЕЧТО`.

```
auto v7 = x; // T = int, ТИП = int
auto v8 = cx; // T = int, ТИП = int
auto v9 = rx; // T = int, ТИП = int
```

## Для ссылок и указателей [00:06]
Если видим `НЕЧТО*` или `НЕЧТО&`:

* Если `value` — ссылка, убери это.
* Сделай pattern-matching типа `value` с `НЕЧТО`, выясни `T`.
  Оно никогда не ссылка.

```
foo(x); // T = int, ТИП = int&
foo(cx); // T = const int, ТИП = const int&
foo(rx); // T = const int, ТИП = const int&
```

`auto`-переменная работает так же:
```
auto &v1 = x; // T = int, ТИП = int&
auto &v2 = cx; // T = const int, ТИП = const int&
auto &v3 = rx; // T = const int, ТИП = const int&
const auto &v4 = x; // T = int, ТИП = const int&
const auto &v5 = cx; // T = int, ТИП = const int&
const auto &v6 = rx; // T = int, ТИП = const int&
```

При этом `const` лежит "за" ссылкой:
```
template<typename T> void bar(const T &x);
bar(x); // T = int, ТИП = const int&
bar(cx); // T = int, ТИП = const int&
bar(rx); // T = int, ТИП = const int&
```

На засыпку:
```
int *a;
int * const b;
const int * c;
const int * const d;
auto v1 = a;  // T=int*, ТИП=int*
auto v2 = b;  // T=int*, ТИП=int*
auto v3 = c;  // T=const int*, ТИП=const int*
auto v4 = d;  // T=const int*, ТИП=const int*
```

# Статические члены классов-константы [00:15]
## Линковка обычных констант [00:05]
* Обычно слово `const` для глобальных переменныех автоматически влечёт
  за собой internal linkage: видно только из текущей единицы трансляции.
* Поэтому можно объявлять константы в заголовке и всё работает без ошибок
  multiple definition.
  ```
  // a.h
  const int x = 10;
  // a.cpp
  #include "a.h"
  // b.cpp
  #include "a.h"  // Забыли перекомпилировать после изменения a.h, упс.
  ```
* Заумность: если не хотим — можно дописать `extern`, будет `extern const int x/* = 10*/;`.
  Но так не надо.

## Старый способ для статических [00:07]
* Напоминание: статические члены классов-переменные: обычно должно объявляться в классе
  и определяться снаружи (причём ровно один раз):
  ```
  // h
  struct A {
      static int x;
  };
  // cpp
  int A::x = 10;
  ```
* Константы аналогично:
  ```
  // h
  struct A {
      /*static*/ const int y;
      static const char * /*const*/ s1;
  };
  // cpp
  const int A::y = 10;
  const char* const A::s1 = "foo";
  ```
* Но это не всегда удобно.
  * Надо писать константу не там, где объявлена, даже если это просто число.
  * Компилятору сложнее оптимизировать константы в других единицах трансляции.
  * Нельзя использовать этот `const` для размеров массивов (GCC сможет, потому что
    он умеет в variable length array, VLA):
    ```
    int arr[A::y];
    ```
* Если у нас статическая константа целочисленного типа или enum,
  то можно проинициализировать константной прямо в объявлении.
  ```
  // h
  struct A {
      static const int y = 10;
  };
  ```
  Тогда можно использовать в константных выражаниях.
  * Но если ODR-used, то всё равно нужно ровно одно определение.
  * Пример, когда не нужно: заиспользовали как размер массива: `int arr[A::y];`
  * Пример, когда нужно: взяли адрес: `foo(&A::y);`
  * Пример, когда нужно: инициализируется сложной функцией: `static const int y = foo();`
* Заумность: с C++17 можно сделать inline-переменную (отключит ODR),
  получим `static inline const`, но лучше не надо.

## Хороший способ [00:03]
* Любую константу пробуете сделать `constexpr` вместо `const`.
  Это не часть типа (но автоматически влечёт `const`),
  это дополнительное ограничения компилятору: вычисли на этапе компиляции.
* В противном случае честно объявляете в `.h` и определяете в `.cpp`.

# Ещё дальше
* Stateful metaprogramming (только практики?): https://stackoverflow.com/questions/44267673/is-stateful-metaprogramming-ill-formed-yet
* precompiled header
* Кого читать про C++:
  * ISO C++ FAQ
  * CoreCppGuidelines
  * GotW (Herb Sutter)
  * Andrei Alexandrescu
  * Scott Meyers
  * TotW (Google)
