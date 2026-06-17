module;
#include "LinkSet.hpp"
export module SetOperator;
import Types;

export enum class OpState : UInt8 {
    INSERT, DELETE, REPLACE, FIND, INTERSECT, UNION, DIFFERENCE, SYMMETRIC_DIFF
};

export enum class OpTarget : UInt8 {
    SET_A, SET_B, NONE
};

export struct OptContext {
    LinkSet<Int32>& setA;
    LinkSet<Int32>& setB;
    LinkSet<Int32>& setOptResult;
    OpState& opState;
    OpTarget& opTarget;
    String opData;
    List<Int32>& findElement;
};

List<Int32> split(const String& str) {
    List<Int32> result;
    Int32 num;
    Boolean inNumber = false;

    for (const char c : str) {
        if (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
            inNumber = true;
        } else if (inNumber) {
            result.push_back(num);
            num = 0;
            inNumber = false;
        }
    }
    if (inNumber) {
        result.push_back(num);
    }
    return result;
}

export namespace SetOperator {
    void executeOperation(OptContext ctx) {
        LinkSet<Int32>& targetSet = ctx.opTarget == OpTarget::SET_A ? ctx.setA : ctx.setB;

        const List<Int32> elements = split(ctx.opData);
        switch (ctx.opState) {
            case OpState::INSERT: {
                for (Int32 e : elements) {
                    targetSet.insert(e);
                }
                break;
            }
            case OpState::DELETE: {
                for (Int32 e : elements) {
                    targetSet.remove(e);
                }
                break;
            }
            case OpState::REPLACE: {
                targetSet.clear();
                for (Int32 e : elements) {
                    targetSet.insert(e);
                }
                break;
            }
            case OpState::FIND: {
                ctx.findElement.clear();
                for (Int32 e : elements) {
                    ctx.findElement.push_back(e);
                }
                break;
            }
            case OpState::INTERSECT: {
                ctx.setOptResult.clear();
                auto intersectionSet = LinkSet<Int32>::intersectionSet(ctx.setA, ctx.setB);
                ctx.setOptResult.insertAll(intersectionSet);
                break;
            }
            case OpState::UNION: {
                ctx.setOptResult.clear();
                auto unionSet = LinkSet<Int32>::unionSet(ctx.setA, ctx.setB);
                ctx.setOptResult.insertAll(unionSet);
                break;
            }
            case OpState::DIFFERENCE: {
                ctx.setOptResult.clear();
                auto differenceSet = LinkSet<Int32>::differenceSet(ctx.setA, ctx.setB);
                ctx.setOptResult.insertAll(differenceSet);
                break;
            }
            case OpState::SYMMETRIC_DIFF: {
                ctx.setOptResult.clear();
                auto symmetricDifferenceSet = LinkSet<Int32>::symmetricDifference(ctx.setA, ctx.setB);
                ctx.setOptResult.insertAll(symmetricDifferenceSet);
                break;
            }
            default: break;
        }
    }
    void executeOperation(LinkSet<Int32>& setA, LinkSet<Int32>& setB, OpState opState, OpTarget opTarget, const String& opData, LinkSet<Int32>& setOptResult) {

    }
}