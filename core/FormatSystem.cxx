#include <iostream>
#include <fstream>

#include "TString.h"

#include "Point.h"
#include "Source.h"
#include "System.h"
#include "FormatSystem.h"

using ygse::Point;
using ygse::Source;
using ygse::System;
using ygse::FormatSystem;

using std::cout;
using std::endl;

FormatSystem::FormatSystem() :fmt("%.3f") {
    /* ... */
}

void FormatSystem::LoadSystem(int i, System sys) {
    systems[i] = sys;
}

void FormatSystem::Print(const char* outname) {
    Print(outname, "Say something", "what");
}

void FormatSystem::SetFormat(const char* fmt) {
    this->fmt = fmt;
}

const char* FormatSystem::GetFormattedFloat(double val) {
    return TString::Format(fmt, val).Data();
}

void FormatSystem::Print(const char* outname, const char* caption, const char* label) {
    std::ofstream fout;
    fout.open(outname);
    // landscape env.
    fout << "\\begin{landscape}\n";
    // header
    fout << "\\begin{table}[]\n\\begin{tabular}{cc|ccccccccc}\n";
    // title row
    fout << "\\multicolumn{2}{c|}{" << systems[0].GetTag1() << "}";
    fout << "\t& 0-5\\%";
    fout << "\t& 5-10\\%";
    fout << "\t& 10-20\\%";
    fout << "\t& 20-30\\%";
    fout << "\t& 30-40\\%";
    fout << "\t& 40-50\\%";
    fout << "\t& 50-60\\%";
    fout << "\t& 60-70\\%";
    fout << "\t& 70-80\\%\t\\\\ \\hline\n";
    // default rows
    fout << "\\multirow{2}{*}{" << systems[0].GetTag2() << "} & value ";
    for (int isys=0; isys<nCent; isys++) {
        fout << "\t & " << GetFormattedFloat(systems[isys].GetDefaultValue());
    }
    fout << "\t \\\\\n";
    fout << "\t & stat. error ";
    for (int isys=0; isys<nCent; isys++) {
        fout << "\t & " << GetFormattedFloat(systems[isys].GetDefaultError());
    }
    fout << "\t \\\\\n";
    fout << "\t & sys. error ";
    for (int isys=0; isys<nCent; isys++) {
        fout << "\t & " << GetFormattedFloat(systems[isys].GetSigma());
    }
    fout << "\t \\\\ \\hline\n";
    // varied sources
    int nvrd = systems[0].GetN();
    for (int ivrd=0; ivrd<nvrd; ivrd++) {
        int ncuts = systems[0].GetN(ivrd);
        fout << "\\multirow{" << ncuts+1 << "}{*}";
        fout << "{" << systems[0].GetSysErrTag(ivrd) << "}";
        for (int icut=0; icut<ncuts; icut++) {
            fout << "\t&" << systems[0].GetSysErrTag(ivrd, icut);
            for (int isys=0; isys<nCent; isys++) {
                fout << "\t & " << GetFormattedFloat(systems[isys].GetSysErrValue(ivrd, icut));
            }
            fout << "\\\\\n";
        }
        fout << "\t& $\\Sigma$";
        for (int isys=0; isys<nCent; isys++) {
            fout << "\t & " << GetFormattedFloat(systems[isys].GetSysErrValue(ivrd));
        }
        if (ivrd == nvrd-1) {
            fout << "\n";
        } else {
            fout << "\\\\ \\hline\n";
        }
    }
    fout << "\\end{tabular}\n\\caption{" << caption << "}\n\\label{tab:" << label << "}\n\\end{table}\n";
    fout << "\\end{landscape}\n";
    fout.close();
}