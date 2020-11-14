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
    std::vector<std::pair<T*,T*>> GetCollisions() const;
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
            return double(std::abs(remaining_position - cut_position)) / double(std::min(remaining_position, cut_position)) + double(cut_object * 2) /* + cut_centerness*/;
        }
        static constexpr int obj_limit = 10;
        std::vector<std::shared_ptr<T>> mObjs;
        std::array<std::unique_ptr<KDNode>, 2> mChildren;
        int mDivAxis = -1;
        double mDivVal = std::numeric_limits<double>::quiet_NaN();
        void Balance()
        {
            if(mObjs.size() < obj_limit) return;
            // if(mObjs.empty()) return;
            auto& dd = DebugDrawer::Get();
            assert(mDivAxis == -1);
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
            // if(mObjs.size() < obj_limit) return;
            double best_penalty = Penalty(objsize, objsize, 0.0, mObjs.size());
            for(int dim=0; dim<Dimensions; ++dim)
            {
                for(const auto& bc : best_cuts_dim[dim])
                {
                    const double p = Penalty(bc.first, bc.second.first, 2.0 * std::abs(bc.second.second - Avg[dim]) / (Max[dim]-Min[dim]), objsize);
                    if(p < best_penalty)
                    {
                        best_penalty = p;
                        // printf("new best cut %c %d<- %d ->%d score %f\n", dim ? 'y' : 'x', bc.second.first, bc.first, objsize - bc.second.first - bc.first, p);
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
            while(!mObjs.empty())
            {
                std::shared_ptr<T> obj = mObjs.back();
                mObjs.pop_back();
                const auto& bb = obj->GetBoundingBox();
                if(bb.first[mDivAxis] < mDivVal)
                {
                    mChildren[0]->mObjs.push_back(obj);
                }
                if(bb.second[mDivAxis] > mDivVal)
                {
                    mChildren[1]->mObjs.push_back(obj);
                }
            }
            for(auto& c : mChildren)
            {
                c->Balance();
            }
        }
        std::vector<std::pair<T*,T*>> Collide()
        {
            std::vector<std::pair<T*,T*>> ret;
            if(!mObjs.empty())
            {
                for(int i=0; i<mObjs.size()-1; ++i)
                {
                    const auto& io = mObjs[i].get();
                    for(int j=i+1; j<mObjs.size(); ++j)
                    {
                        const auto& jo = mObjs[j].get();
                        if(io->Collide(jo))
                        {
                            ret.emplace_back(io,jo);
                        }
                    }
                }
            }
            if(mDivAxis == -1) return ret;
            for(const auto& c : mChildren)
            {
                auto ch_ret = c->Collide();
                ret.insert(ret.end(), ch_ret.begin(), ch_ret.end());
            }
            return ret;
        }
    };
    std::unique_ptr<KDNode> mRoot;
};

template<typename T, size_t D>
template<typename It>
void KDTree<T, D>::Create(It begin, It end)
{
    ZoneScopedN("Geometry_2DT::BroadPhase");
    mRoot.reset(new KDNode);
    auto& objs = mRoot->mObjs;
    objs.insert(objs.end(), begin, end);
    mRoot->Balance();
}

template<typename T, size_t D>
std::vector<std::pair<T*,T*>> KDTree<T, D>::GetCollisions() const
{
    ZoneScopedN("Geometry_2DT::NarrowPhase");
    return mRoot->Collide();
}

class Geometry_2DT : public KDTree<Object,2>, public IGeometry
{
public:
    virtual ~Geometry_2DT() {}
    virtual void DoCollisions(std::vector<std::shared_ptr<Object> > os) override
    {
        auto& dd = DebugDrawer::Get();
        Create(os.begin(), os.end());
        auto colls = GetCollisions();
        typedef std::pair<Object*,std::shared_ptr<PhysEffect>> objeff;
        typedef std::pair<objeff,objeff> interaction;
        std::vector<interaction> colleffs;
        for (auto c : colls)
        {
            dd.DrawLine(c.first->GetPosition(), c.second->GetPosition());
            objeff a(c.first,c.first->DoCollision(c.second));
            objeff b(c.second,c.second->DoCollision(c.first));
            colleffs.push_back(interaction(a,b));
        }
        for (auto i : colleffs)
        {
            i.first.first->DoEffect(i.second.second);
            i.second.first->DoEffect(i.first.second);
        }
    }
};

#endif
