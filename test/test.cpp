#include <gtest/gtest.h>
#include "libcpp-pg-pool.hpp"
#include <string>

using namespace lklibs;

TEST(PgPoolTest, SimpleConnection)
{
    const auto connectionString = "dbname=my_db user=my_user password=my_password host=localhost port=5432";

    PgPool pool(connectionString, 10);

    auto dbConnection = pool.acquire();

    EXPECT_TRUE(dbConnection->is_open());

    pqxx::work txn(*dbConnection);

    pqxx::result res = txn.exec("SELECT * FROM my_table");

    for (const auto& row : res)
    {
        std::cout << row[0].c_str() << std::endl;
    }
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

TEST(PgPoolTest, SharedConnection)
{
    const auto connectionString = "dbname=my_db user=my_user password=my_password host=localhost port=5432";

    PgPool pool(connectionString, 10);

    {
        SampleConsumer myConsumer{};

        {
            const auto dbConnection = pool.acquire();

            EXPECT_TRUE(dbConnection->is_open());

            myConsumer.setDbConnection(dbConnection);

            myConsumer.queryData();
        }

        EXPECT_TRUE(myConsumer.isConnectionOpen());
    }
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
