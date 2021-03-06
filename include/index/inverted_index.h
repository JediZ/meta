/**
 * @file inverted_index.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INVERTED_INDEX_H_
#define META_INVERTED_INDEX_H_

#include <queue>
#include <stdexcept>

#include "index/disk_index.h"
#include "index/make_index.h"

namespace meta
{

namespace corpus
{
class corpus;
class document;
}

namespace index
{

template <class>
class chunk_handler;

template <class, class>
class postings_data;
}
}

namespace meta
{
namespace index
{

/**
 * The inverted_index class stores information on a corpus indexed by term_ids.
 * Each term_id key is associated with a per-document frequency (by doc_id).
 *
 * It is assumed all this information will not fit in memory, so a large
 * postings file containing the (term_id -> each doc_id) information is saved on
 * disk. A lexicon (or "dictionary") contains pointers into the large postings
 * file. It is assumed that the lexicon will fit in memory.
 */
class inverted_index : public disk_index
{
  public:
    /**
     * Basic exception for inverted_index interactions.
     */
    class inverted_index_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

    using primary_key_type = term_id;
    using secondary_key_type = doc_id;
    using postings_data_type = postings_data<term_id, doc_id>;
    using index_pdata_type = postings_data<std::string, doc_id>;
    using exception = inverted_index_exception;

    /**
     * inverted_index is a friend of the factory method used to create
     * it.
     */
    template <class Index, class... Args>
    friend std::shared_ptr<Index> make_index(const std::string&, Args&&...);

    /**
     * inverted_index is a friend of the factory method used to create
     * cached versions of it.
     */
    template <class Index, template <class, class> class Cache, class... Args>
    friend std::shared_ptr<cached_index<Index, Cache>>
        make_index(const std::string& config_file, Args&&... args);

  protected:
    /**
     * @param config The table that specifies how to create the
     * index.
     */
    inverted_index(const cpptoml::table& config);

  public:
    /**
     * Move constructs a inverted_index.
     */
    inverted_index(inverted_index&&);

    /**
     * Move assigns a inverted_index.
     */
    inverted_index& operator=(inverted_index&&);

    /**
     * inverted_index may not be copy-constructed.
     */
    inverted_index(const inverted_index&) = delete;

    /**
     * inverted_index may not be copy-assigned.
     */
    inverted_index& operator=(const inverted_index&) = delete;

    /**
     * Default destructor.
     */
    virtual ~inverted_index();

    /**
     * @param doc The document to tokenize
     */
    void tokenize(corpus::document& doc);

    /**
     * @param t_id The term_id to search for
     * @return the postings data for a given term_id
     */
    virtual std::shared_ptr<postings_data_type>
        search_primary(term_id t_id) const;

    /**
     * @param t_id The term to search for
     * @return the document frequency of a term (number of documents it
     * appears in)
     */
    uint64_t doc_freq(term_id t_id) const;

    /**
     * @param t_id The term_id to search for
     * @param d_id The doc_id to search for
     */
    uint64_t term_freq(term_id t_id, doc_id d_id) const;

    /**
     * @return the total number of terms in this index
     */
    uint64_t total_corpus_terms();

    /**
     * @param t_id The specified term
     * @return the number of times the given term appears in the corpus
     */
    uint64_t total_num_occurences(term_id t_id) const;

    /**
     * @return the average document length in this index
     */
    double avg_doc_length();

  private:
    /**
     * This function initializes the disk index; it is called by the
     * make_index factory function.
     * @param config_file The configuration to be used
     */
    void create_index(const std::string& config_file);

    /**
     * This function loads a disk index from its filesystem
     * representation.
     */
    void load_index();

    /**
     * @return whether this index contains all necessary files
     */
    bool valid() const;

    /// Forward declare the implementation
    class impl;
    /// Implementation of this index
    util::pimpl<impl> inv_impl_;
};
}
}

#endif
