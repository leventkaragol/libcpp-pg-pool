#include "libcpp-pg-pool.hpp"
#include <string>
#include <iostream>

using namespace lklibs;

void simpleConnectionSample()
{
    const auto connectionString = "dbname=my_db user=my_user password=my_password host=localhost port=5432";

    // IMPORTANT: Connection Pool should be created only once in the application
    // It may take up to 1 second to be ready depending on the pool size
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
}

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

void sharedConnectionSample()
{
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
}

int main()
{
    simpleConnectionSample();

    sharedConnectionSample();

    return 0;
}
