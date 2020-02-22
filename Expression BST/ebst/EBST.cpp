#include "EBST.h"

EBST::EBST(const std::string& expressionString) {

}

std::string EBST::toString(EBST::OutputType type) const {
    return std::string();
}

std::vector<ExpressionResult> EBST::calculateResult() const {
    return std::vector<ExpressionResult>();
}

void EBST::insert(const ExpressionNode& key, const bool &) {

}

EBST::NodePtr EBST::insert(EBST::NodePtr& node, const AbstractBST::KVPair& keyValue) {
    return EBST::NodePtr();
}

bool EBST::remove(const ExpressionNode& key) {
    return false;
}

EBST::NodePtr EBST::remove(EBST::NodePtr& p, const ExpressionNode& key) {
    return EBST::NodePtr();
}

EBST::NodePtr EBST::find(const EBST::NodePtr& node, const ExpressionNode& key) const {
    return EBST::NodePtr();
}

EBST::Node::Node(const AbstractBST::KVPair& keyValue) : AbstractNode(keyValue) {

}

EBST::NodePtr EBST::Node::next() const {
    return EBST::NodePtr();
}
