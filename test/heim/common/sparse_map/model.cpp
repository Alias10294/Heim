#include "support.hpp"
using namespace heim_test::sparse_map;

namespace {
template<typename Map>
void run_model(std::uint32_t seed, unsigned steps, unsigned domain)
{
  Map actual;
  std::unordered_map<typename Map::key_type, typename Map::mapped_type> expected;
  rng random{seed};
  for(unsigned step=0;step<steps;++step)
  {
    auto key=static_cast<typename Map::key_type>(random.bounded(domain));
    int value=static_cast<int>(random.next()%200001u)-100000;
    unsigned const op = random.bounded(9);
    INFO("seed=" << seed << " step=" << step << " op=" << op
         << " key=" << static_cast<std::uint64_t>(key) << " domain=" << domain);
    switch(op)
    {
      case 0: case 1:
      {
        auto [it,added]=actual.try_emplace(key,value);
        auto [oracle_it,oracle_added]=expected.try_emplace(key,value);
        REQUIRE(added==oracle_added);
        REQUIRE(it->second==oracle_it->second);
        break;
      }
      case 2:
      {
        auto [it,added]=actual.insert_or_assign(key,value);
        auto [oracle_it,oracle_added]=expected.insert_or_assign(key,value);
        REQUIRE(added==oracle_added);
        REQUIRE(it->second==oracle_it->second);
        break;
      }
      case 3: case 4:
      {
        REQUIRE(actual.try_erase(key)==(expected.erase(key)!=0));
        break;
      }
      case 5:
      {
        REQUIRE(actual.contains(key)==expected.contains(key));
        break;
      }
      case 6:
      {
        auto it=actual.find(key);
        REQUIRE((it!=actual.end())==expected.contains(key));
        if(expected.contains(key)) REQUIRE(it->second==expected.at(key));
        break;
      }
      case 7:
      {
        if(expected.contains(key))
        {
          actual.at(key)=value;
          expected.at(key)=value;
        }
        else
        {
          REQUIRE_THROWS_AS(actual.at(key),std::out_of_range);
        }
        break;
      }
      case 8:
      {
        if(random.bounded(73)==0)
        {
          actual.clear();expected.clear();
        }
        else
        {
          auto c=actual;
          verify(c, expected, domain);
        }
        break;
      }
    }
    // Check the mutated or queried key immediately on every step.
    REQUIRE(actual.contains(key) == expected.contains(key));
    if (expected.contains(key)) REQUIRE(actual.at(key) == expected.at(key));
    if(step%113==0) verify(actual, expected, domain);
  }
  verify(actual, expected, domain);
}
}

TEST_CASE("sparse_map randomized model baseline seed", "[common][sparse_map][model]")
{ run_model<map<>>(0xDEADBEEFu,7500,420); }

TEST_CASE("sparse_map randomized model different seed", "[common][sparse_map][model]")
{ run_model<map<>>(0xF00DBABEu,7500,420); }

TEST_CASE("sparse_map randomized model clustered keys", "[common][sparse_map][model]")
{ run_model<map<>>(0x12345678u,6000,24); }

TEST_CASE("sparse_map randomized model broad sparse keys", "[common][sparse_map][model]")
{ run_model<map<>>(0x9E3779B9u,6500,6000); }

TEST_CASE("sparse_map randomized model all keys collide in one page", "[common][sparse_map][model]")
{ run_model<map<unsigned,int,64>>(0xCAFEBABEu,5000,60); }

TEST_CASE("sparse_map randomized model single element pages", "[common][sparse_map][model]")
{ run_model<map<unsigned,int,1>>(0x13579BDFu,5000,180); }

TEST_CASE("sparse_map randomized model two element pages", "[common][sparse_map][model]")
{ run_model<map<unsigned,int,2>>(0x2468ACE0u,5000,180); }

TEST_CASE("sparse_map randomized model contiguous sparse index", "[common][sparse_map][model]")
{ run_model<map<unsigned,int,0>>(0x01020304u,5000,360); }

TEST_CASE("sparse_map randomized model uint8 keys", "[common][sparse_map][model]")
{ run_model<map<std::uint8_t,int,8>>(0xA5A5A5A5u,9000,180); }

TEST_CASE("sparse_map randomized model uint16 keys", "[common][sparse_map][model]")
{ run_model<map<std::uint16_t,int,16>>(0x5A5A5A5Au,7500,4000); }

TEST_CASE("sparse_map dense key/value association stable across repeated copy swap", "[common][sparse_map][model]")
{
  map<> a,b;
  std::unordered_map<unsigned,int> expect_a,expect_b;
  for(unsigned i=0;i<128;++i)
  {
    a.insert_or_assign(i*5u,int(i)); expect_a[i*5u]=int(i);
    b.insert_or_assign(i*7u,int(i+100)); expect_b[i*7u]=int(i+100);
  }
  for(unsigned round=0;round<25;++round)
  {
    auto clone=a;
    verify(clone,expect_a);
    a.swap(b); expect_a.swap(expect_b);
    auto copy=b;
    b=copy;
    verify(a,expect_a);verify(b,expect_b);
  }
}

TEST_CASE("sparse_map randomized iterator deletion follows reference model", "[common][sparse_map][model]")
{
  map<> m;
  std::unordered_map<unsigned,int> oracle;
  for(unsigned k=0;k<300;++k) { m.try_emplace(k*3u,int(k));oracle.emplace(k*3u,int(k)); }
  rng random{0xABCD1234u};
  while(!m.empty())
  {
    auto at=static_cast<std::ptrdiff_t>(random.bounded(static_cast<std::uint32_t>(m.size())));
    auto it=m.begin()+at;
    unsigned key=it->first;
    auto after=m.erase(it);
    REQUIRE(after==m.begin()+at);
    REQUIRE(oracle.erase(key)==1);
    if(m.size()%29==0) verify(m,oracle);
  }
  verify(m,{});
}

TEST_CASE("sparse_map massive sequential insert and reverse removal", "[common][sparse_map][model]")
{
  map<> m;
  for(unsigned i=0;i<2500;++i) REQUIRE(m.try_emplace(i*4u,int(i*2)).second);
  for(unsigned i=2500;i>0;--i)
  {
    auto key=(i-1)*4u;
    REQUIRE(m.at(key)==int((i-1)*2));
    REQUIRE(m.try_erase(key));
  }
  REQUIRE(m.empty());
}
