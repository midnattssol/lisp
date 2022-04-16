
/* Check if the parentheses in a string are balanced. */
bool has_balanced_parentheses(std::string token, char lparen, char rparen) {
    auto size = token.size();
    auto depths = get_paren_depths(token, lparen, rparen);
    for (auto depth : depths) {
        if (depth < 0) return false;
    }
    return not depths[size - 1];
}

/* Get the parenthesis depths across a string. */
std::vector<int> get_paren_depths(std::string token, char lparen, char rparen) {
    std::vector<int> output;
    int accumulator = 0;

    for (auto character : token) {
        accumulator += (character == lparen) - (character == rparen);
        output.push_back(accumulator);
    }
    return output;
}
