/*

Thread-safe, high performance, PostgreSQL connection pooling library for C++ (17+)
version 1.0.0
https://github.com/leventkaragol/libcpp-pg-pool

If you encounter any issues, please submit a ticket at https://github.com/leventkaragol/libcpp-pg-pool/issues

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

*/

#ifndef LIBCPP_PG_POOL_HPP
#define LIBCPP_PG_POOL_HPP

#include <pqxx/pqxx>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <string>
#include <utility>

namespace lklibs
{
    class PgPool
    {
    public:
        explicit PgPool(std::string connectionString, const size_t poolSize = 100): connectionString(std::move(connectionString)), poolSize(poolSize), stop(false)
        {
            for (size_t i = 0; i < poolSize; ++i)
            {
                pool.push(createConnection());
            }
        }

        ~PgPool()
        {
            std::unique_lock<std::mutex> lock(mtx);

            stop = true;

            connectionAvailable.notify_all();

            while (!pool.empty())
            {
                auto conn = std::move(pool.front());

                pool.pop();

                conn.reset();
            }
        }

        std::shared_ptr<pqxx::connection> acquire()
        {
            std::unique_lock<std::mutex> lock(mtx);

            connectionAvailable.wait(lock, [this]() { return !pool.empty() || stop; });

            if (stop)
            {
                throw std::runtime_error("Connection pool is shutting down");
            }

            auto conn = std::move(pool.front());

            pool.pop();

            return std::shared_ptr<pqxx::connection>(conn.release(), [this](pqxx::connection* dbConnection)
            {
                returnConnection(dbConnection);
            });
        }

    private:
        std::unique_ptr<pqxx::connection> createConnection()
        {
            return std::make_unique<pqxx::connection>(connectionString);
        }

        void returnConnection(pqxx::connection* conn)
        {
            std::unique_lock<std::mutex> lock(mtx);

            pool.push(std::unique_ptr<pqxx::connection>(conn));

            connectionAvailable.notify_one();
        }

        std::string connectionString;
        size_t poolSize;
        std::queue<std::unique_ptr<pqxx::connection>> pool;
        std::mutex mtx;
        std::condition_variable connectionAvailable;
        bool stop;
    };
}

#endif // LIBCPP_PG_POOL_HPP
