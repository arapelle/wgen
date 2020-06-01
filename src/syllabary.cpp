#include <wgen/syllabary.hpp>
#include <experimental/random>
#include <locale>
#include <random>
#include <algorithm>

namespace wgen
{
////////////////////////////////////////////////////////////////////////////////

Syllabary::Syllabary(const std::string_view &consonants, const std::string_view &vowels, const std::string_view &codas)
    : consonants_(consonants.begin(), consonants.end()),
      vowels_(vowels.begin(), vowels.end()), codas_(codas.begin(), codas.end())
{
}

Syllabary::Syllabary(std::vector<char> consonants, std::vector<char> vowels, std::vector<char> codas)
    : consonants_(std::move(consonants)), vowels_(std::move(vowels)), codas_(std::move(codas))
{
}

std::string Syllabary::random_word(unsigned word_length, Format format) const
{
    std::string word;
    word.resize(word_length);

    random_word_(&*word.begin(), &*word.end(), word_length, format);

    return word;
}

strn::string64 Syllabary::random_word64(unsigned word_length, Format format) const
{
    strn::string64 word;
    word_length = std::min<unsigned>(word_length, strn::string64::max_length());
    random_word_(&*word.begin(), &*std::next(word.begin(), word_length), word_length, format);
    return word;
}

std::string Syllabary::format_word(std::string_view format_str, const char c_char, const char v_char, Format format) const
{
    std::string word;
    word.reserve(format_str.length());

    auto iter = format_str.begin();
    for (auto end_iter = codas_.empty() ? format_str.end() : format_str.end() - 1;
         iter < end_iter;
         ++iter)
    {
        char ch = *iter;
        if (ch == c_char)
            word.push_back(random_consonant_());
        else if (ch == v_char)
            word.push_back(random_vowel_());
        else
            word.push_back(ch);
    }
    if (iter < format_str.end())
    {
        char ch = *iter;
        if (ch == c_char)
            word.push_back(random_coda_());
        else if (ch == v_char)
            word.push_back(random_vowel_());
        else
            word.push_back(ch);
    }

    if (!word.empty())
        format_(&*word.begin(), &*word.end(), format);

    return word;
}

strn::string64 Syllabary::format_word64(std::string_view format_str, const char c_char, const char v_char, Format format) const
{
    return strn::string64(format_word(format_str, c_char, v_char, format));
}

std::size_t Syllabary::number_of_possible_words(unsigned word_length) const
{
    if (word_length > 0)
    {
        std::size_t nos2 = consonants_.size() * vowels_.size();
        std::size_t result = 1;
        for (std::size_t number_of_syllables = word_length / 2; number_of_syllables > 0; --number_of_syllables)
            result *= nos2;
        if (word_length & 1)
            result *= codas_.size() + vowels_.size();
        return result;
    }
    return 0;
}

// private

void Syllabary::random_word_(char* first, char* last, unsigned word_length, Format format) const
{
    if (word_length > 0)
    {
        char* iter = first;

        if ((word_length & 1) && (codas_.empty() || std::experimental::randint(0,1) == 1))
        {
            *iter++ = random_vowel_();
        }
        for (std::size_t i = 0, end_i = word_length / 2; i < end_i; ++i)
        {
            *iter++ = random_consonant_();
            *iter++ = random_vowel_();
        }
        if (iter != last)
        {
            *iter++ = codas_.empty() ? random_consonant_() : random_coda_();
        }

        format_(first, last, format);
    }
}

void Syllabary::format_(char* first, char* last, Format format) const
{
    auto& facet = std::use_facet<std::ctype<std::string::value_type>>(std::locale());
    switch (format)
    {
    case Format::Name:
    {
        char& first_ch = *first;
        first_ch = facet.toupper(first_ch);
        std::for_each(std::next(first), last, [&facet](std::string::value_type& ch){ ch = facet.tolower(ch); });
        break;
    }
    case Format::Lower:
    {
        std::for_each(first, last, [&facet](std::string::value_type& ch){ ch = facet.tolower(ch); });
        break;
    }
    case Format::Upper:
    {
        std::for_each(first, last, [&facet](std::string::value_type& ch){ ch = facet.toupper(ch); });
        break;
    }
    case Format::No_format:
    default:
        ;
    }
}

char Syllabary::random_consonant_() const
{
    std::size_t idx = std::experimental::randint<std::size_t>(0, consonants_.size()-1);
    return consonants_[idx];
}

char Syllabary::random_vowel_() const
{
    std::size_t idx = std::experimental::randint<std::size_t>(0, vowels_.size()-1);
    return vowels_[idx];
}

char Syllabary::random_coda_() const
{
    std::size_t idx = std::experimental::randint<std::size_t>(0, codas_.size()-1);
    return codas_[idx];
}

}
