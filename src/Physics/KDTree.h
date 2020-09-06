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
        std::vector<std::shared_ptr<T>> mObjs;
        std::pair<std::unique_ptr<KDNode>,std::unique_ptr<KDNode>> mChildren;
        int mDivAxis = -1;
        double mDivVal = std::numeric_limits<double>::quiet_NaN();
        void Balance()
        {
            //TODO: when
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
