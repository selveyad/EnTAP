//
// Created by harta on 3/29/17.
//

#ifndef ENTAP_QUERYSEQUENCE_H
#define ENTAP_QUERYSEQUENCE_H
#include <iostream>


class QuerySequence {
    public:
        bool operator>(const QuerySequence& querySequence);
        QuerySequence(std::string,std::string,std::string,std::string,float);
        QuerySequence();

        float getE_val() const;
        void setE_val(float e_val);
        const std::string &getDatabase_path() const;
        void setDatabase_path(const std::string &database_path);
        const std::string &getQseqid() const;
        void setQseqid(const std::string &qseqid);
        const std::string &getSseqid() const;
        void setSseqid(const std::string &sseqid);
        const std::string &getStitle() const;
        void setStitle(const std::string &stitle);

private:
        friend std::ostream& operator<<(std::ostream& , const QuerySequence&);
        bool contaminant;
        int tax_id;
        float e_val;
        std::string database_path, qseqid,sseqid, stitle, species;
public:
    bool isContaminant() const;

    void setContaminant(bool contaminant);

    int getTax_id() const;

    void setTax_id(int tax_id);

public:
    const std::string &getSpecies() const;

    void setSpecies(const std::string &species);


};


#endif //ENTAP_QUERYSEQUENCE_H