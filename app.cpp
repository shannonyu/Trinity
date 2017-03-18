#include "exec.h"
#include "google_codec.h"
#include "indexer.h"
#include "playground_index_source.h"
#include "terms.h"
#include "segment_index_source.h"
#include <set>
#include <text.h>
#include "merge.h"
#include "lucene_codec.h"
#include <crypto.h>


using namespace Trinity;

#if 0
int main(int argc, char *argv[])
{
	MergeCandidatesCollection collection;
	int fd = open("/tmp/TSEGMENTS/1500/terms.data", O_RDONLY);

	require(fd != -1);
	auto fileSize = lseek64(fd, 0, SEEK_END);
	auto fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	require(fileData != MAP_FAILED);
	
	IndexSourcePrefixCompressedTermsView itv({(const uint8_t *)fileData, (uint32_t)fileSize});
	fd = open("/tmp/TSEGMENTS/1500/index", O_RDONLY);
	require(fd != -1);
	fileSize = lseek64(fd, 0, SEEK_END);
	fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	require(fileData != MAP_FAILED);

	Trinity::Codecs::Lucene::AccessProxy ap("/tmp/TSEGMENTS/1500/", (uint8_t *)fileData);

	SegmentTerms st("/tmp/TSEGMENTS/1500/");

	collection.insert({1500, &itv, &ap, {}});


	std::vector<std::pair<str8_t, Trinity::term_index_ctx>> terms;
	simple_allocator allocator;
	auto outSession = new Trinity::Codecs::Lucene::IndexSession("/tmp/TSEGMENTS/1800/");


	collection.commit();
	outSession->begin();
	collection.merge(outSession, &allocator, &terms);
	outSession->end();

	SLog("positionsOut.size = ", outSession->positionsOut.size(), "\n");

	require(outSession->indexOut.size() == fileSize);
	SLog("indexOut.size = ", outSession->indexOut.size(), "\n");
	delete outSession;



	return 0;
}
#endif

#if 0
int main()
{
	matched_document document;
	auto qt = [](const uint16_t pos)
	{
		auto mqt = new matched_query_term();
		auto ptr = (query_term_instances *)malloc(sizeof(query_term_instances) + sizeof(query_term_instances::instance_struct));

		ptr->instances[0].index = pos;
		mqt->queryTermInstances = ptr;
		return *mqt;
	};
	matched_query_term terms[] =
	{
		qt(10),
		qt(5),
		qt(1),
		qt(2),
		qt(50),
		qt(8),
		qt(100),
		qt(102),
		qt(80),
		qt(81),
		qt(85),
		qt(70),
		qt(150),
		qt(250),
		qt(15),
		qt(1000),
		qt(28),
	};

	document.matchedTermsCnt = sizeof_array(terms);
	document.matchedTerms = terms;

	document.sort_by_query_index();

	for (uint32_t i{0}; i != sizeof_array(terms); ++i)
	{
		Print(terms[i].queryTermInstances->instances[0].index, "\n");
	}

	return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	simple_allocator allocator;
	const strwlen32_t in(argv[1]);
	Trinity::query q(in);

	q.process_runs(true, true, [&q](auto &v)
	{
		for (uint32_t i{0}; i != v.size(); ++i)
		{
			const auto it = v[i];

			if (it->p->size == 1 && it->p->terms[0].token.Eq(_S("jump")))
			{
				*it = *ast_parser("(jump OR hop OR leap)"_s32, q.allocator).parse();
			}
                        else if (i +3 <= v.size() && it->p->size == 1 && it->p->terms[0].token.Eq(_S("world")) 
				&& v[i+1]->p->size == 1 && v[i+1]->p->terms[0].token.Eq(_S("of"))
				&& v[i+2]->p->size == 1 && v[i+2]->p->terms[0].token.Eq(_S("warcraft")))
			{
				query::replace_run(v.data() + i, 3, ast_parser("(wow OR (world of warcraft))"_s32, q.allocator).parse());
			}
			else if (it->p->size == 1 && it->p->terms[0].token.Eq(_S("puppy")))
			{
				*it = *ast_parser("puppy OR (kitten OR kittens OR cat OR cats OR puppies OR dogs OR pets)"_s32, q.allocator).parse();
			}
		}
	});

	q.normalize();
	Print(q, "\n");

	return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
	auto sess = std::make_unique<Trinity::Codecs::Lucene::IndexSession>("/tmp/LUCENE/1/");
	//auto sess = std::make_unique<Trinity::Codecs::Google::IndexSession>("/tmp/LUCENE/1/");
	auto enc = sess->new_encoder();
	term_index_ctx tctx;
	term_hit hits[8192];
	uint32_t pos{1};

        enc->begin_term();

#if 1
	const size_t n{15};

        enc->begin_document(1, n);
        for (uint32_t i{10}; i != 10 + n; ++i)
        {
		enc->new_hit(pos++, {(uint8_t *)&i, sizeof(uint32_t)});
	}
	enc->end_document();

	enc->begin_document(10, 2);
	enc->new_hit(15, {});
	enc->new_hit(16, {});
	enc->end_document();

	enc->begin_document(100, 2);
	enc->new_hit(25, {});
	enc->new_hit(50, {});
	enc->end_document();

#else
        for (uint32_t i{1}; i < 250; ++i)
        {
                if (i > 100 && i < 125)
                {
                        enc->begin_document(i, 2);
                        enc->new_hit(pos++, {(uint8_t *)&i, sizeof(uint32_t)});
                        enc->new_hit(pos++, {});
                        enc->end_document();
                }
                else
                {
                        enc->begin_document(i, 1);
                        enc->new_hit(pos++, {(uint8_t *)&i, sizeof(uint32_t)});
                        enc->end_document();
                }

                if (i < 15)
                        ++i;
        }
#endif
        enc->end_term(&tctx);

	SLog(sess->indexOut.size(), "\n");
        delete enc;



	Print("\n\n\n\n\n");

        auto ap = new Trinity::Codecs::Lucene::AccessProxy("/tmp/LUCENE/1/", (uint8_t *)sess->indexOut.data(), (uint8_t *)sess->positionsOut.data());
	auto dec = ap->new_decoder(tctx);
	DocWordsSpace dws;

	dec->begin();
	dec->seek(1);
	//if (!dec->seek(80)) { SLog("Failed to seek\n"); } dec->seek(51000);

	if (dec->curDocument.id == MaxDocIDValue)
	{
		SLog("Done\n");
		dec->seek(5550000);

		return 0;
	}

	for (;;)
	{
		const auto freq = dec->curDocument.freq;

		Print(ansifmt::color_magenta, "DOCUMENT:", dec->curDocument.id, " ", freq, ansifmt::reset, "\n");
		dec->materialize_hits(0, &dws, hits);

		for (uint32_t i{0}; i != freq; ++i)
		{
			Print("HIT:     ", hits[i].pos,  " PAYLOAD:", *(uint32_t *)&hits[i].payload, "\n");
		}


		if (dec->next() == false)
			break;
	}

	delete dec;
	delete ap;
        return 0;
}
#endif


#if 1
int main(int argc, char *argv[])
{
	if (argc == 1)
        {
                int fd = open("/home/system/Data/BestPrice/SERVICE/clusters.data", O_RDONLY | O_LARGEFILE);

                require(fd != -1);

                const auto fileSize = lseek64(fd, 0, SEEK_END);
                auto fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
                simple_allocator a;
                SegmentIndexSession indexSess;

                close(fd);
                require(fileData != MAP_FAILED);

		Print("INDEXING\n");
                for (const auto *p = static_cast<const uint8_t *>(fileData), *const e = p + fileSize; p != e;)
                {
                        p += sizeof(uint16_t);
                        const auto chunkSize = *(uint32_t *)p;

                        p += sizeof(uint32_t);

                        for (const auto chunkEnd = p + chunkSize; p != chunkEnd;)
                        {
                                const auto id = *(uint32_t *)p;
                                p += sizeof(uint32_t);
                                strwlen8_t title((char *)p + 1, *p);
                                p += title.size() + sizeof(uint8_t);
                                ++p;
                                const auto n = *(uint16_t *)p;
                                p += sizeof(uint16_t);

                                p += n * sizeof(uint32_t);

                                title.p = a.CopyOf(title.data(), title.size());

                                auto d = indexSess.begin(id);
                                uint16_t pos{1};

                                for (const auto *p = title.p, *const e = p + title.size(); p != e;)
                                {
                                        if (const auto len = Text::TermLengthWithEnd(p, e))
                                        {
                                                auto mtp = (char *)p;

                                                for (uint32_t i{0}; i != len; ++i)
                                                        mtp[i] = Buffer::UppercaseISO88597(p[i]);

                                                const strwlen8_t term(p, len);
						uint32_t hit{25121561};

                                                d.insert(term, pos, {(uint8_t *)&hit, sizeof(uint32_t)});
                                                //d.insert(term, 0, {});
                                                ++pos;

                                                p += len;
                                        }
                                        else
                                        {
                                                if (!isspace(*p))
                                                        pos += 2;
                                                ++p;
                                        }
                                }

                                indexSess.update(d);
                        }
                }
                munmap(fileData, fileSize);

#if 0
                auto is = new Trinity::Codecs::Google::IndexSession("/tmp/TSEGMENTS/100");

                indexSess.commit(is);
#else
                auto is = new Trinity::Codecs::Lucene::IndexSession("/tmp/TSEGMENTS/1500");

                indexSess.commit(is);

#endif
                delete is;
        }
	else
	{
		std::vector<uint32_t> maskedProducts{6908848};
		IOBuffer maskedProductsBuf;

		pack_updates(maskedProducts, &maskedProductsBuf);
		auto updates = unpack_updates({(uint8_t *)maskedProductsBuf.data(), maskedProductsBuf.size()});
		auto maskedDocsSrc = new TrivialMaskedDocumentsIndexSource(updates);
		IndexSourcesCollection sources;
		Buffer asuc;

                if (false)
                {
                        auto ss = new SegmentIndexSource("/tmp/TSEGMENTS/100");

                        sources.insert(ss);
                        ss->Release();
                }

                if (true)
                {
                        auto ss = new SegmentIndexSource("/tmp/TSEGMENTS/1500");
                        sources.insert(ss);
                        ss->Release();
                }

#if 0
		{
			MergeCandidatesCollection collection;
			simple_allocator a;
			std::vector<std::pair<str8_t, term_index_ctx>> outTerms;
			auto outSess = new Trinity::Codecs::Lucene::IndexSession("/tmp/TSEGMENTS/8192/");

			for (const auto it : sources.sources)
			{
				auto ss = (SegmentIndexSource *)it;
				auto tv = ss->segment_terms()->new_terms_view();

                                collection.insert({ss->generation(), tv, ss->access_proxy(), ss->masked_documents()});
                        }

			collection.commit();

			Print("MERGING\n");
			collection.merge(outSess, &a, &outTerms);
			outSess->persist_terms(outTerms);

			std::vector<uint32_t> dids;
			persist_segment(outSess, dids);
			return 0;
		}
#endif


                //sources.insert(maskedDocsSrc);
		maskedDocsSrc->Release();


		sources.commit();

#if 0
		{
			const auto ud = sources.sources.back()->masked_documents();
			auto reg = masked_documents_registry::make(&ud, 1);

			Print(reg->test(7512491), "\n");
			return 0;
		}
#endif



		auto filter = std::make_unique<MatchedIndexDocumentsFilter>();


		struct BPFilter final
			: public MatchedIndexDocumentsFilter
		{
			CRC32Generator crc32;

			~BPFilter()
			{
				SLog("CRC32 = ", crc32.get(), "\n");
			}
		
                        ConsiderResponse consider(matched_document &doc) override final
                        {
				doc.sort_matched_terms_by_query_index();
#if 1
				const auto n = doc.matchedTermsCnt;
				size_t curRun{0};


				//SLog("MATCHED ", doc.id, " https://www.bestprice.gr/item/", doc.id|(1<<31), "\n");
				for (uint32_t i{0}; i != n; ++i)
				{
					const auto t = doc.matchedTerms + i;

					//Print("Matched term ", t->queryTermInstances->term.id, " ", t->queryTermInstances->term.token, " ", t->hits->freq, " freq\n");

					for (uint32_t i{0}; i != t->hits->freq; ++i)
					{
						const auto &hit = t->hits->all[i];

						//Print(">> ", hit.pos, " ", hit.payloadLen, " => ", *(uint32_t *)&hit.payload, "\n");

						crc32.update(hit.pos);
						crc32.update(hit.payloadLen);
						crc32.update((uint8_t *)&hit.payload, hit.payloadLen);
					}
					continue;

					for (uint32_t i{0}; i != t->queryTermInstances->cnt; ++i)
					{
						const auto &it = t->queryTermInstances->instances[i];

						Print("INSTANCE ", it.index, ", ", it.rep, ", ", it.flags, "\n");
					}

					if (i)
					{
						bool seq{false};

						for (uint32_t k{0}; k != t->queryTermInstances->cnt; ++k)
						{
							const auto idx = t->queryTermInstances->instances[k].index;

							if (idx)
							{
								if (auto ptr = queryIndicesTerms[idx - 1])
								{
									for (uint32_t j{0}; j != ptr->cnt; ++j)
									{
										const auto id = ptr->termIDs[j];

										for (uint32_t l{0}; l != t->hits->freq; ++l)
										{
											if (const auto pos = t->hits->all[l].pos)
											{
												if (dws->test(id, pos -1))
													seq = true;
											}
										}
									}
								}
							}
						}

                                                if (seq)
                                                {
							++curRun;
						}
						else if (curRun)
						{
							SLog("Ending run ", curRun, "\n");
							
							curRun = 0;
						}
					}

				}
#endif
				return ConsiderResponse::Continue;

			}
		};

		asuc.append(argv[1]);

		for (uint32_t i{0}; i != asuc.size(); ++i)
			asuc.data()[i] = Buffer::UppercaseISO88597(asuc.data()[i]);

		query q(asuc.AsS32());

#if 1
		auto node = ast_node::make(q.allocator, ast_node::Type::BinOp);
		auto specialTokensNode = ast_node::make(q.allocator, ast_node::Type::ConstTrueExpr);

		specialTokensNode->expr = ast_parser("(LEGEND OR ZELDA)"_s32, q.allocator).parse();
		specialTokensNode->set_alltokens_flags(1);

		node->binop.rhs = q.root;
		node->binop.lhs = specialTokensNode;
		node->binop.op = Operator::AND;

		q.root = node;
#endif



		


		
		exec_query<MatchedIndexDocumentsFilter>(q, &sources);
		//auto res = exec_query<BPFilter>(q, &sources);
	}

        return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
	{
                SegmentIndexSession sess;

                const auto index_document = [](auto &documentSess, const strwlen32_t input) {
			uint32_t pos{1};

                        for (const auto *p = input.data(), *const e = p + input.size(); p != e;)
                        {
                                if (const auto len = Text::TermLengthWithEnd(p, e))
                                {
					documentSess.insert(strwlen8_t(p, len), pos++);
					p+=len;
                                }
                                else
				{
					if (!isblank(*p))
						++pos;
                                        ++p;
				}
                        }
                };

                {
                        auto doc = sess.begin(1);

			index_document(doc, "world of warcraft mists of pandaria is the most successful MMORPG ever created"_s32);
			//index_document(doc, "the"_s32);
                        sess.update(doc);
                }

                {
                        auto doc = sess.begin(2);

			index_document(doc, "lord of the rings the return of the king. an incredible film about hobits, rings and powerful wizards in the mythical middle earth"_s32);
			//index_document(doc, "the"_s32);
                        sess.update(doc);
                }

                auto is = new Trinity::Codecs::Google::IndexSession("/tmp/TSEGMENTS/1/");

                sess.commit(is);

                delete is;
        }


#if 0
	{
		auto ss = new SegmentIndexSource("/tmp/TSEGMENTS/1/");
		auto maskedDocuments = masked_documents_registry::make(nullptr, 0);
		query q("apple");

		exec_query(q, ss, maskedDocuments);

		free(maskedDocuments);
		ss->Release();
	}
#else
	{
		query q(argv[1]);
		IndexSourcesCollection bpIndex;
		auto ss = new SegmentIndexSource("/tmp/TSEGMENTS/1/");

		bpIndex.insert(ss);
		ss->Release();
	
		bpIndex.commit();

		exec_query<MatchedIndexDocumentsFilter>(q, &bpIndex);
	}
#endif

        return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
        const auto index_document = [](auto &documentSess, const strwlen32_t input) {
                uint32_t pos{1};

                for (const auto *p = input.data(), *const e = p + input.size(); p != e;)
                {
                        if (const auto len = Text::TermLengthWithEnd(p, e))
                        {
                                documentSess.insert(strwlen8_t(p, len), pos++);
                                p += len;
                        }
                        else
                        {
                                if (!isblank(*p))
                                        ++pos;
                                ++p;
                        }
                }
        };


        {
                SegmentIndexSession sess;

                {
                        auto doc = sess.begin(1);

                        index_document(doc, "world of warcraft mists of pandaria is the most successful MMORPG ever created"_s32);
                        sess.update(doc);
                }

                {
                        auto doc = sess.begin(2);

                        index_document(doc, "lord of the rings the return of the king. an incredible film about hobits, rings and powerful wizards in the mythical middle earth"_s32);
                        sess.update(doc);
                }

                auto is = new Trinity::Codecs::Google::IndexSession("/tmp/TSEGMENTS/1/");

                sess.commit(is);
                delete is;
        }


        {
                SegmentIndexSession sess;

                {
                        auto doc = sess.begin(1);

                        index_document(doc, "world of warcraft mists of pandaria is the most successful MMORPG ever created"_s32);
                        sess.update(doc);
                }


                auto is = new Trinity::Codecs::Google::IndexSession("/tmp/TSEGMENTS/2/");

                sess.commit(is);
                delete is;
        }


	int fd = open("/tmp/TSEGMENTS/1/terms.data", O_RDONLY|O_LARGEFILE);
	require(fd != -1);
	auto fileSize = lseek64(fd, 0, SEEK_END);
	auto fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	const range_base<const uint8_t *, uint32_t> termsData1((uint8_t *)fileData, uint32_t(fileSize));
	IndexSourcePrefixCompressedTermsView tv1(termsData1);

	fd = open("/tmp/TSEGMENTS/1/index", O_RDONLY|O_LARGEFILE);
	require(fd != -1);
	fileSize = lseek64(fd, 0, SEEK_END);
	fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	auto ap1 = new Trinity::Codecs::Google::AccessProxy("/tmp/TSEGMENTS/1/", (uint8_t *)fileData);

	fd = open("/tmp/TSEGMENTS/2/terms.data", O_RDONLY|O_LARGEFILE);
	require(fd != -1);
	fileSize = lseek64(fd, 0, SEEK_END);
	fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	const range_base<const uint8_t *, uint32_t> termsData2((uint8_t *)fileData, uint32_t(fileSize));
	IndexSourcePrefixCompressedTermsView tv2(termsData2);

	fd = open("/tmp/TSEGMENTS/2/index", O_RDONLY|O_LARGEFILE);
	require(fd != -1);
	fileSize = lseek64(fd, 0, SEEK_END);
	fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	auto ap2 = new Trinity::Codecs::Google::AccessProxy("/tmp/TSEGMENTS/2/", (uint8_t *)fileData);

	
	MergeCandidatesCollection collection;

	collection.insert({1, &tv1, ap1, {}});
	collection.insert({2, &tv2, ap2, {}});
	collection.commit();

	Print("====================================================================\n");

#if 0
	{
		while (!tv1.done())
		{
			Print(tv1.cur().first, "\n");
			tv1.next();
		}

		Print("-- --- -- -- - - - ----\n");
		while (!tv2.done())
		{
			Print(tv2.cur().first, "\n");
			tv2.next();
		}
		return 0;
	}
#endif

	auto is = new Trinity::Codecs::Google::IndexSession("/tmp/TSEGMENTS/3");
	simple_allocator allocator;
	std::vector<std::pair<strwlen8_t, term_index_ctx>> terms;

	collection.merge(is, &allocator, &terms);

	SLog(size_repr(is->indexOut.size()), "\n");



        return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
	std::vector<std::pair<strwlen8_t, term_index_ctx>> terms;
	const strwlen32_t allTerms(_S("world of warcraft amiga 1200 apple iphone ipad macbook pro imac ipod edge zelda gamecube playstation psp nes snes gameboy sega nintendo atari commodore ibm"));
	IOBuffer index, data;
	Switch::vector<terms_skiplist_entry> skipList;
	simple_allocator allocator;

	for (const auto it : allTerms.Split(' '))
	{
		const term_index_ctx tctx{1, {uint32_t(it.data() - allTerms.data()), it.size()}};
		
		terms.push_back({{it.data(), it.size()}, tctx});
	}

	pack_terms(terms, &data, &index);

	unpack_terms_skiplist({(uint8_t *)index.data(), index.size()}, &skipList, allocator);

	for (uint32_t i{1}; i != argc; ++i)
        {
                const strwlen8_t q(argv[i]);
                const auto res = lookup_term({(uint8_t *)data.data(), data.size()}, q, skipList);

                Print(q , " => ", res.indexChunk, "\n");
        }

	for (const auto &&it : terms_data_view({(uint8_t *)data.data(), data.size()}))
	{
		Print("[", it.first, "] => [", it.second.documents, "]\n");
	}

        return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
	int fd = open("/home/system/Data/BestPrice/SERVICE/clusters.data", O_RDONLY|O_LARGEFILE);

	require(fd != -1);

	const auto fileSize = lseek64(fd, 0, SEEK_END);
	auto fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
	simple_allocator a;
	std::set<strwlen8_t> uniq;
	std::vector<std::pair<strwlen8_t, term_index_ctx>> terms;
	size_t termsLenSum{0};

	close(fd);
	require(fileData != MAP_FAILED);

	for (const auto *p = static_cast<const uint8_t *>(fileData), *const e = p + fileSize; p != e; )
	{
		p+=sizeof(uint16_t);
		const auto chunkSize = *(uint32_t *)p;

		p+=sizeof(uint32_t);

		for (const auto chunkEnd = p + chunkSize; p != chunkEnd; )
		{
			p+=sizeof(uint32_t);
			strwlen8_t title((char *)p + 1, *p); p+=title.size() + sizeof(uint8_t);
			++p;
			const auto n = *(uint16_t *)p; p+=sizeof(uint16_t);

			p+=n * sizeof(uint32_t);

			title.p = a.CopyOf(title.data(), title.size());


			for (const auto *p = title.p, *const e = p+ title.size(); p != e; )
                        {
                                if (const auto len = Text::TermLengthWithEnd(p, e))
                                {
					auto mtp = (char *)p;

					for (uint32_t i{0}; i != len; ++i)
						mtp[i] = Buffer::UppercaseISO88597(p[i]);
					
					const strwlen8_t term(p, len);

					if (uniq.insert(term).second)
					{
						terms.push_back({term, { 1, {0, 0} }});
						termsLenSum+=term.size();
					}
	

					p+=len;
                                }
                                else
                                        ++p;
                        }
                }
	}
	munmap(fileData, fileSize);
	// 574,584 distinct terms across all cluster titles



	SLog(terms.size(), "\n"); 	



	IOBuffer index, data;
	Switch::vector<terms_skiplist_entry> skipList;
	simple_allocator allocator;
	uint64_t before;


	before = Timings::Microseconds::Tick();
	pack_terms(terms, &data, &index);	  // Took 2.923s to pack 79.01kb 5.74mb 4.06mb (We 'd need about 14MB without prefix compression and varint encoding of term_index_ctx)
	SLog("Took ", duration_repr(Timings::Microseconds::Since(before)), " to pack ", size_repr(index.size()), " ", size_repr(data.size()), " ", size_repr(termsLenSum), "\n");

	before = Timings::Microseconds::Tick();
	unpack_terms_skiplist({(uint8_t *)index.data(), index.size()}, &skipList, allocator); 	// Took 0.002s to unpack 4489
	SLog("Took ", duration_repr(Timings::Microseconds::Since(before)), " to unpack ", skipList.size(), "\n");

	for (uint32_t i{1}; i != argc; ++i)
        {
                const strwlen8_t q(argv[i]);
		const auto before = Timings::Microseconds::Tick();
                const auto res = lookup_term({(uint8_t *)data.data(), data.size()}, q, skipList);
		const auto t = Timings::Microseconds::Since(before); // 3 to 26us 

                Print(q , " => (", res.documents, ", ",  res.indexChunk, ") in ", duration_repr(t), "\n"); 
        }
        return 0;
}
#endif


#if 0
int main(int argc, char *argv[])
{
        // Inex
        SegmentIndexSession indexerSess;

        {
                auto proxy = indexerSess.begin(1);

                proxy.insert("apple"_s8, 1);
                proxy.insert("macbook"_s8, 2);
                proxy.insert("pro"_s8, 3);

                indexerSess.insert(proxy);
        }

        {
                auto proxy = indexerSess.begin(2);

                proxy.insert("apple"_s8, 1);
                proxy.insert("iphone"_s8, 2);

                indexerSess.insert(proxy);
        }

        Trinity::Codecs::Google::IndexSession codecIndexSess("/tmp/segment_1/");

        indexerSess.commit(&codecIndexSess);

        // Search
        int fd = open("/tmp/segment_1/index", O_RDONLY | O_LARGEFILE);

        require(fd != -1);
        const auto fileSize = lseek64(fd, 0, SEEK_END);
        auto fileData = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);

        close(fd);
        require(fileData != MAP_FAILED);

        auto ap = new Trinity::Codecs::Google::AccessProxy("/tmp/segment_1/", (uint8_t *)fileData);
        auto seg = Switch::make_sharedref<Trinity::segment>(ap);
        auto maskedDocsReg = masked_documents_registry::make(nullptr, 0);

        exec_query(Trinity::query("apple"_s32), *seg.get(), maskedDocsReg);

        return 0;
}
#endif

#if 0
int main(int argc, char *argv[])
{
	Trinity::Codecs::Google::IndexSession sess("/tmp/");
	Trinity::Codecs::Google::Encoder encoder(&sess);
	term_index_ctx appleTCTX, iphoneTCTX, crapTCTX;


	sess.begin();

	encoder.begin_term();

	encoder.begin_document(10, 2);
	encoder.new_position(1);
	encoder.new_position(2);
	encoder.end_document();

	encoder.begin_document(11, 5);
	encoder.new_position(15);
	encoder.new_position(20);
	encoder.new_position(21);
	encoder.new_position(50);
	encoder.new_position(55);
	encoder.end_document();

	encoder.begin_document(15, 1);
	encoder.new_position(20);
	encoder.end_document();


	encoder.begin_document(25, 1);
	encoder.new_position(18);
	encoder.end_document();


	encoder.begin_document(50,1);
	encoder.new_position(20);
	encoder.end_document();

	encoder.end_term(&appleTCTX);



	// iphone
	encoder.begin_term();
	
	encoder.begin_document(11, 1);
	encoder.new_position(51);
	encoder.end_document();

	encoder.begin_document(50, 1);
	encoder.new_position(25);
	encoder.end_document();

	encoder.end_term(&iphoneTCTX);


	// crap
	encoder.begin_term();
	
	encoder.begin_document(25, 1);
	encoder.new_position(1);
	encoder.end_document();
	encoder.end_term(&crapTCTX);

	sess.end();



	Print(" ============================================== DECODING\n");

#if 0

        {
                range_base<const uint8_t *, uint32_t> range{(uint8_t *)sess.indexOut.data(), appleTCTX.chunkSize};
		term_index_ctx tctx;
                Codecs::Google::IndexSession mergeSess("/tmp/foo");
                Codecs::Google::Encoder enc(&mergeSess);
		auto maskedDocuments = masked_documents_registry::make(nullptr, 0);

                enc.begin_term();
                mergeSess.merge(&range, 1, &enc, maskedDocuments);
                enc.end_term(&tctx);

                SLog(sess.indexOut.size(), " ", mergeSess.indexOut.size(), "\n");
                return 0;
        }
#endif

#if 0
	Trinity::Codecs::Google::Decoder decoder;

	decoder.init(tctx, (uint8_t *)indexData.data(), nullptr); //(uint8_t *)encoder.skipListData.data());

	SLog("chunk size= ", tctx.chunkSize, "\n");

	decoder.begin();
#if 0
	decoder.seek(2);
	decoder.seek(28);
	decoder.seek(50);
	decoder.seek(501);
#endif

	while (decoder.cur_document() != UINT32_MAX)
	{
		Print(ansifmt::bold, "document ", decoder.cur_document(), ansifmt::reset,"\n");
		decoder.next();
	}

	return 0;
#endif


	std::unique_ptr<Trinity::Codecs::Google::AccessProxy> ap(new Trinity::Codecs::Google::AccessProxy("/tmp/", (uint8_t *)sess.indexOut.data()));
	auto idxSrc = Switch::make_sharedref<PlaygroundIndexSource>(ap.release());

	idxSrc->tctxMap.insert({"apple"_s8, appleTCTX});
	idxSrc->tctxMap.insert({"iphone"_s8, iphoneTCTX});
	idxSrc->tctxMap.insert({"crap"_s8, crapTCTX});


	//query q("apple OR iphone NOT crap"_s32);
	query q("\"apple iphone\""_s32);
	auto maskedDocumentsRegistry = masked_documents_registry::make(nullptr, 0);

	exec_query(q, idxSrc.get(), maskedDocumentsRegistry);


        return 0;
}
#endif
