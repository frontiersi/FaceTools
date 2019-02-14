#include <iostream>
#include <unordered_set>
#include <vector>
#include <sol.hpp>
using IntSet = std::unordered_set<int>;

namespace N1 { namespace N2 {

class MC
{
public:
    MC( double x, int n) : _vals(n, x) {}

    const std::vector<double>& vals() const { return _vals;}
private:
    std::vector<double> _vals;
};  // end class

class TC
{
public:
    TC( double x, int n) : _mc(x, n)
    {
        for ( int i = n; i < 2*n; ++i)
            _iset.insert(i);
    }   // end ctor

    const MC* get() const { return &_mc;}

    const IntSet& iset() const { return _iset;}

private:
    MC _mc;
    IntSet _iset;
};  // end class

} }   // end namespaces


int main()
{
    sol::state lua;
    lua.open_libraries( sol::lib::base);
    lua.open_libraries( sol::lib::math);

    using N1::N2::TC;
    using N1::N2::MC;

    lua.new_usertype<MC>( "MC",
            "vals", &MC::vals);

    lua.new_usertype<TC>( "TC",
            "get", &TC::get,
            "iset", &TC::iset);

    lua.script( R"(
            lt = [[
sdfsdfsdf
xvxvxvcxc
qwqwee]]
            function tfunc( tc)
                a = tc:get():vals()[1]
                b = tc:get():vals()[2]
                v = a + b

                if ( v > 40) then
                    print( "Over 40")
                elseif ( v > 30) then
                    print( "Over 30")
                end
                print(lt)

                -- Check values from tc's IntSet returned from iset
                iset = tc:iset()
                print( #iset .. " values")
                for i, j in iset:pairs() do
                    print( "Set has " .. i .. " " .. j)
                end

                return v
            end
    )");
    sol::function flua = lua["tfunc"];

    const TC tc(23, 7);
    const int v = flua( &tc);
    std::cerr << "Written from C++: " << v << std::endl;

    return 0;
}   // end main


    /*
    lua.script( R"(
            function f (a)
                return a+5
            end
    )");

    sol::function f = lua["f"];
    int y = f(20);
    std::cerr << y << std::endl;
    */

    /*
    lua.open_libraries( sol::lib::base);
    bool loadedOkay = false;
    try
    {
        lua.safe_script_file( "m8.lua");
        std::cerr << "Loaded script" << std::endl;
        loadedOkay = true;
    }   // end try
    catch( const sol::error& e)
    {
        std::cerr << " *** " << e.what() << std::endl;
    }   // end catch

    if ( !loadedOkay)
    {
        std::cerr << "Failed to load and execute script!" << std::endl;
        return -1;
    }   // end if


    auto table = lua["mc"];
    std::cerr << (table.valid()) << std::endl;

    int id, ndps, dims;
    std::string name, desc, catg, prms;
    if ( table.valid())
    {
        id = table["id"].get_or(-1);
        ndps = table["ndps"].get_or(1);
        dims = table["dims"].get_or(-1);

        if ( sol::optional<std::string> v = table["name"])
            name = v.value();
        if ( sol::optional<std::string> v = table["desc"])
            desc = v.value();
        if ( sol::optional<std::string> v = table["catg"])
            catg = v.value();
        if ( sol::optional<std::string> v = table["prms"])
            prms = v.value();

        std::cerr << name << std::endl;
        std::cerr << "dims = " << dims << std::endl;
        std::cerr << "prms = " << prms << std::endl;

        if ( sol::optional<sol::table> v = table["stats"])
        {
            sol::table stats = v.value();
            for ( int i = 1; i <= stats.size(); ++i)
            {
                if ( sol::optional<sol::table> v = stats[i])
                {
                    sol::table d = v.value();
                    if ( sol::optional<std::string> v = d["ethn"])
                        std::cerr << v.value() << std::endl;
                    if ( sol::optional<std::string> v = d["sexs"])
                        std::cerr << v.value() << std::endl;

                    if ( sol::optional<sol::table> v = d["data"])
                    {
                        sol::table data = v.value();
                        for ( int j = 1; j <= dims; ++j)
                        {
                            sol::table dimj = data[j];
                            for ( int k = 1; k <= dimj.size(); ++k)
                            {
                                double s0 = dimj[k][1];
                                double s1 = dimj[k][2];
                                double s2 = dimj[k][3];
                                std::cerr << s0 << " = {" << s1 << ", " << s2 << "}" << std::endl;
                            }   // end for
                        }   // end for
                    }   // end if
                }   // end if
            }   // end for
        }   // end if
    }   // end if
*/
