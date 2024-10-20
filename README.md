# libcpp-pg-pool

Thread-safe, high performance, PostgreSQL connection pooling library for C++ (17+)

[![linux](https://github.com/leventkaragol/libcpp-pg-pool/actions/workflows/linux.yml/badge.svg)](https://github.com/leventkaragol/libcpp-pg-pool/actions/workflows/linux.yml)
[![windows](https://github.com/leventkaragol/libcpp-pg-pool/actions/workflows/windows.yml/badge.svg)](https://github.com/leventkaragol/libcpp-pg-pool/actions/workflows/windows.yml)


> [!TIP]
> Please read this document before using the library. I know, you don't have time but reading
> this document will save you time. I mean just this file, it's not long at all. Trial and error
> will cost you more time.

# Table of Contents

* [How to add it to my project](#how-to-add-it-to-my-project)
* [How to use? (Simple Connection)](#how-to-use-simple-connection)
* [How to use? (Shared Connection)](#how-to-use-shared-connection)
* [Semantic Versioning](#semantic-versioning)
* [Full function list](#full-function-list)
* [License](#license)
* [Contact](#contact)

## How to add it to my project?

This is a header only library. So actually, all you need is to add the libcpp-pg-pool.hpp file
in src folder to your project and start using it with #include.

But this library is uses libpqxx under the hood. So, you also need to add libpqxx to
your project before to use it.

You can find usage examples in the examples folder, also find a sample CMakeLists.txt file content below.

```cmake
cmake_minimum_required(VERSION 3.14)

project(myProject)

find_package(libpqxx CONFIG REQUIRED)

add_executable(myProject main.cpp libcpp-pg-pool.hpp)

target_link_libraries(myProject PRIVATE libpqxx::pqxx)

```

## How to use? (Simple Connection)

You can use the **"acquire"** method to obtain a new connection. The value returned from the acquire method
is a shared pointer and the connection is automatically returned to the pool as soon as it goes out of scope.
Therefore, you do not need to close the connection manually.

> [!IMPORTANT]
> Connection Pool should be created only once in the application
> It may take up to 1 second to be ready depending on the pool size

```cpp
#include "libcpp-pg-pool.hpp"

using namespace lklibs;

int main() {

    const auto connectionString = "dbname=my_db user=my_user password=my_password host=localhost port=5432";

    // You can optionally specify the connection pool size, but if you don't, the default value will be 100
    PgPool pool(connectionString, 10);


    {
        // Acquire a connection from the pool
        const auto dbConnection = pool.acquire();

        pqxx::work txn(*dbConnection);

        const auto result = txn.exec("SELECT * FROM my_table");

        for (const auto& row : result)
        {
            std::cout << row[0].c_str() << std::endl;
        }
    }

    // Connection is automatically returned to the pool when the dbConnection goes out of scope
    // So, no need to manually close the connection

    return 0;
}
```

> [!IMPORTANT]
> The connection pool size should not exceed PostgreSQL's max_connections limit,
> otherwise you will get an error like "too many clients"

## How to use? (Shared Connection)

The return value from the acquire method is a shared pointer and counts shared references. When you open a connection
and share it with another class, even if the acquired connection object goes out of scope, it keeps the connection open
until the last shared class object is destroyed.

```cpp
#include "libcpp-pg-pool.hpp"

using namespace lklibs;

class SampleConsumer
{
public:
    void setDbConnection(std::shared_ptr<pqxx::connection> dbConnection)
    {
        this->dbConnection = std::move(dbConnection);
    }

    void queryData() const
    {
        pqxx::work txn(*dbConnection);

        const auto result = txn.exec("SELECT * FROM my_table");

        for (const auto& row : result)
        {
            std::cout << row[0].c_str() << std::endl;
        }
    }

    [[nodiscard]] bool isConnectionOpen() const
    {
        return dbConnection->is_open();
    }

private:
    std::shared_ptr<pqxx::connection> dbConnection;
};

int main() {

    const auto connectionString = "dbname=my_db user=my_user password=my_password host=localhost port=5432";

    PgPool pool(connectionString);


    {
        SampleConsumer myConsumer{};

        {
            // Acquire a connection from the pool
            const auto dbConnection = pool.acquire();

            // Connection is shared with SampleConsumer
            myConsumer.setDbConnection(dbConnection);

            myConsumer.queryData();
        }

        // Connection is still alive even after the dbConnection goes out of scope because SampleConsumer is still holding the shared_ptr

        std::cout << "Connection status:" << myConsumer.isConnectionOpen() << std::endl;
    }

    // Connection is automatically returned to the pool when all shared_ptr holding the connection go out of scope

    return 0;
```

## Semantic Versioning

Versioning of the library is done using conventional semantic versioning. Accordingly,
in the versioning made as **MAJOR.MINOR.PATCH**;

**PATCH:** Includes possible Bug&Fixes and improvements. You definitely want to get this.

**MINOR:** Additional functionality added via backwards compatibility. You probably want to
get this, it doesn't hurt.

**MAJOR:** Additional functionality that breaks backwards compatibility. You'll need to know
what's changed before you get it, and you'll probably have to make changes to your own code.
If I publish something like this, I will definitely add the changes required for migration
section to the documentation.

## Full function list

You can find the complete list of functions in the library below.

> [!TIP]
> All functions and parameters descriptions are also available within the code as comment for IDEs.

```cpp
std::shared_ptr<pqxx::connection> acquire();
```

## License

MIT License

Copyright (c) 2024 Levent KARAGÖL

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contact

If you have problems regarding the library, please open an
[issue on GitHub](https://github.com/leventkaragol/libcpp-pg-pool/issues/new).
Please describe your request, issue, or question in as much detail as possible
and also include the version of your compiler and operating system, as well as
the version of the library you are using. Before opening a new issue, please
confirm that the topic is not already exists in closed issues.