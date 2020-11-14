#pragma once
#ifndef PHYSICS_KDTREE_H_INCLUDED
#define PHYSICS_KDTREE_H_INCLUDED

#include "Basics.h"
#include "IGeometry.h"
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <cassert>
#include <limits>
#include <locale>
#include <map>

constexpr int intpow(int b, int e)
{
    int ret = 1;
    for(int i=0; i<e; ++i) ret*=b;
    return ret;
}

static bool g_written = false;
static int g_nodes = 0;

template<typename T, size_t Dimensions>
class KDTree
{
public:
    KDTree() noexcept {}
    template<typename Iterator>
    KDTree(Iterator begin, Iterator end)
    {
        Create(begin, end);
    }
    void Clear() noexcept
    {
        mRoot.release();
    }
    bool Empty() const { return !mRoot; }
    template<typename Iterator>
    void Create(Iterator begin, Iterator end);
    template<typename Iterator>
    void Insert(Iterator begin, Iterator end);
    void Insert(std::shared_ptr<T> obj);
private:
    struct KDNode
    {
        static double Penalty(int cut_object, int cut_position, double cut_centerness, int num_objs)
        {
            assert(num_objs >= 0);
            assert(cut_object >= 0);
            assert(cut_position >= 0 && cut_position <= num_objs * 2);
            assert(cut_centerness >= 0.0 && cut_centerness <= 1.0);
            const int remaining_position = num_objs - cut_object - cut_position;
            return double(std::abs(remaining_position - cut_position)) / double(std::min(remaining_position, cut_position)) + double(cut_object ) /* + cut_centerness*/;
        }
        static constexpr int obj_limit = 11;
        std::vector<std::shared_ptr<T>> mObjs;
        std::array<std::unique_ptr<KDNode>, 2> mChildren;
        int mDivAxis = -1;
        double mDivVal = std::numeric_limits<double>::quiet_NaN();
        void Balance()
        {
            if(!g_written) g_nodes++;
            if(mObjs.empty()) return;
            auto& dd = DebugDrawer::Get();
            assert(mDivAxis == -1);
            // if(mObjs.size() < obj_limit) return;
            std::vector<std::pair<Vec3d, int8_t>> bb_vec;
            bb_vec.reserve(mObjs.size());
            for(const auto& o : mObjs)
            {
                const auto& bb = o->GetBoundingBox();
                bb_vec.emplace_back(bb.first, 1);
                bb_vec.emplace_back(bb.second, -1);
            }
            std::array<std::map<int, std::pair<int, double>>, Dimensions> best_cuts_dim;
            const int bbsize = bb_vec.size();
            const int objsize = mObjs.size();
            std::array<double, Dimensions> Min, Max, Avg;
            for(int dim=0; dim<Dimensions; ++dim)
            {
                std::sort(bb_vec.begin(), bb_vec.end(), [dim](const auto& a, const auto& b)
                    {
                        if(a.first[dim] < b.first[dim]) return true;
                        if(a.first[dim] > b.first[dim]) return false;
                        return a.second < b.second;
                    });
                Min[dim] = bb_vec.front().first[dim];
                Max[dim] = bb_vec.back().first[dim];
                Avg[dim] = (Min[dim] + Max[dim]) / 2.0;
                // assert(bb_vec[0].second == 1);
                int inside_count = 0;
                int past_count = 0;
                auto& best_cuts = best_cuts_dim[dim];
                for(int i=0; i<bbsize-1; ++i)
                {
                    const auto& cur = bb_vec[i];
                    const auto& next = bb_vec[i+1];
                    inside_count += cur.second;
                    past_count += cur.second < 0 ? 1 : 0;
                    const double cut = (cur.first[dim]+next.first[dim])/2.0;
                    if(best_cuts.count(inside_count) == 0)
                    {
                        best_cuts.emplace(inside_count, std::make_pair(past_count, cut));
                    }
                    else
                    {
                        auto& best_cut = best_cuts.at(inside_count);
                        const auto goodness = std::abs(past_count * 2 - objsize);
                        const auto best_goodness = std::abs(best_cut.first * 2 - objsize);
                        if((best_goodness > goodness) ||
                            (best_goodness == goodness &&
                                std::abs(best_cut.second - Avg[dim]) > std::abs(cut - Avg[dim])))
                        {
                            best_cut.first = past_count;
                            best_cut.second = cut;
                        }
                    }
                }
            }
            Vec3d ddmin, ddmax;
            for(int i=0; i<Dimensions; ++i)
            {
                ddmin[i] = Min[i];
                ddmax[i] = Max[i];
            }
            dd.DrawBox(ddmin,ddmax);
            if(mObjs.size() < obj_limit) return;
            double best_penalty = Penalty(objsize, objsize, 0.0, mObjs.size());
            // megkeressuk a legjobb cut-ot aminek legalacsonyabb a penalty-ja
            for(int dim=0; dim<Dimensions; ++dim)
            {
                for(const auto& bc : best_cuts_dim[dim])
                {
                    const double p = Penalty(bc.first, bc.second.first, 2.0 * std::abs(bc.second.second - Avg[dim]) / (Max[dim]-Min[dim]), objsize);
                    if(p < best_penalty)
                    {
                        best_penalty = p;
                        if(!g_written) printf("new best cut %c %d<- %d ->%d score %f\n", dim ? 'y' : 'x', bc.second.first, bc.first, objsize - bc.second.first - bc.first, p);
                        mDivAxis = dim;
                        mDivVal = bc.second.second;
                    }
                }
            }
            assert(mDivAxis != -1);
            if(mDivAxis == -1)
                return;
            ddmin[mDivAxis] = mDivVal;
            ddmax[mDivAxis] = mDivVal;
            dd.SetColor({0.,0.,1.,1.});
            dd.DrawLine(ddmin, ddmax);
            dd.SetColor({1.,1.,1.,1.});
            for(auto& c : mChildren)
            {
                c.reset(new KDNode);
            }
            // szetosztjuk az objektumokat a 2 gyerekbe
            decltype(mObjs) leftover;
            while(!mObjs.empty())
            {
                std::shared_ptr<T> obj = mObjs.back();
                mObjs.pop_back();
                const auto& bb = obj->GetBoundingBox();
                if(bb.first[mDivAxis] > mDivVal)
                {
                    mChildren[1]->mObjs.push_back(std::move(obj));
                }
                else if(bb.second[mDivAxis] < mDivVal)
                {
                    mChildren[0]->mObjs.push_back(std::move(obj));
                }
                else
                {
                    // (jo kerdes, hogy amit szejjelvagunk az mind2-be bekeruljon-e)
                    // ha fentrol lefele passzolgatjuk az ojjektumokat, amik tesztelesre varnak
                    leftover.push_back(std::move(obj));
                }
            }
            std::swap(mObjs, leftover);
            ////////////////////DEBUG////////////////////
            if(!mObjs.empty())
            {
            bb_vec.clear();
            bb_vec.reserve(mObjs.size());
            for(const auto& o : mObjs)
            {
                const auto& bb = o->GetBoundingBox();
                bb_vec.emplace_back(bb.first, 1);
                bb_vec.emplace_back(bb.second, -1);
            }
            for(int dim=0; dim<Dimensions; ++dim)
            {
                std::sort(bb_vec.begin(), bb_vec.end(), [dim](const auto& a, const auto& b)
                    {
                        if(a.first[dim] < b.first[dim]) return true;
                        if(a.first[dim] > b.first[dim]) return false;
                        return a.second < b.second;
                    });
                Min[dim] = bb_vec.front().first[dim];
                Max[dim] = bb_vec.back().first[dim];
            }
            for(int i=0; i<Dimensions; ++i)
            {
                ddmin[i] = Min[i];
                ddmax[i] = Max[i];
            }
            dd.SetColor({1.,0.,0.,1.});
            dd.DrawBox(ddmin,ddmax);
            dd.SetColor({1.,1.,1.,1.});
            }
            ////////////////////DEBUG////////////////////
            // mind2 gyereket balanszoljuk
            for(auto& c : mChildren)
            {
                c->Balance();
            }
        }
    };
    std::unique_ptr<KDNode> mRoot;
};

template<typename T, size_t D>
template<typename It>
void KDTree<T, D>::Create(It begin, It end)
{
    mRoot.reset(new KDNode);
    auto& objs = mRoot->mObjs;
    objs.insert(objs.end(), begin, end);
    mRoot->Balance();
    if(!g_written) printf("%d nodes\n", g_nodes);
    g_written = true;
}

class Geometry_2DT : public KDTree<Object,2>, public IGeometry
{
public:
    virtual ~Geometry_2DT() {}
    virtual void DoCollisions(std::vector<std::shared_ptr<Object> > os) override
    {
        Create(os.begin(), os.end());
    }
};

#endif
