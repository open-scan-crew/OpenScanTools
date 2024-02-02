#include "test/SafeDataTests.h"

#include "utils/safe_ptr.h"
#include "models/3d/Graph/AGraphNode.h"
#include "models/3d/Graph/TorusNode.h"

void tl::tests::data::upcastTest()
{
    class A {
    public:
        int a = 4;
        virtual int f() {
            return 3;
        };
    };

    class B {
    public:
        virtual int g() {
            return 5;
        };
    };

    class C : public A, public B
    {
    public:
        C(int i) : param(i) {};

        int f() override {
            return A::f() * param;
        }

        int g() override {
            return B::g() * param;
        }
        int param;
    };

    C* c = new C(10);
    int ff = c->f();
    int gg = c->g();
    A* a = (A*)c; // implicit conversion
    B* b = (B*)c; // implicit conversion
    A* as = static_cast<A*>(c); // does the same as the implicit consersion
    B* bs = static_cast<B*>(c); //    for the upcast way

    C* nc_a = static_cast<C*>(a); // Supposedly OK
    C* nc_b = static_cast<C*>(b); // ???

    std::shared_ptr<C> sh_c = std::make_shared<C>(20);
    std::shared_ptr<A> sh_ac(sh_c);  // OK, ptr<A>, ref_count_obj2<C>
    std::shared_ptr<B> sh_bc(sh_c);  // OK, ptr<B>, ref_count_obj2<C>
    std::shared_ptr<C> sh_cac = static_pointer_cast<C>(sh_ac); // static downcast

    std::shared_ptr<B> sh_b; // nullptr
    std::shared_ptr<C> sh_cb = static_pointer_cast<C>(sh_b); // downcast nullptr ?

    std::unordered_set<SafePtr<A>> set_spa;

    SafePtr<C> sp_c = make_safe<C>(30);
    SafePtr<A> sp_a(sp_c);

    set_spa.insert(sp_c);
    set_spa.insert(sp_a);
    SafePtr<B> sp_b(sp_c);
    SafePtr<C> sp_ca = static_pointer_cast<C>(sp_a);

    delete c;
}

void tl::tests::data::integrityTest(size_t count, int seed)
{
    std::vector<SafePtr<AGraphNode>> datas;
    std::vector<SafePtr<TorusNode>> torus;

    for (int i = 0; i < count; ++i)
    {
        torus.emplace_back(make_safe<TorusNode>(2.0, 1.5, 0.35, 0.0));
        datas.push_back(torus.back());
        ReadPtr<TorusNode> rptr(torus.back().cget());
        WritePtr<TorusNode> wptr(static_write_cast<TorusNode>(datas.back()));
        wptr->addGlobalTranslation(glm::dvec3(0.2, 0.3, 0.5));
        assert(rptr);
    }

    /*
    for (int i = 0; i < c; ++i)
    {
        size_t j = ((i * (c + 1)) >> 2) % c;
        TorusNode* obj = obs.getObject<TorusNode>(ids[j]);
        assert(obj);
        const TorusNode* obj2 = obs.cgetObject<TorusNode>(ids[j]);
        assert(obj2);
    }

    for (TLID id : ids)
    {
        obs.deleteObject(id);
    }
    */
}