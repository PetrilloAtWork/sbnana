import ROOT
import os
from array import array
ROOT.PyConfig.IgnoreCommandLineOptions = True
ROOT.gStyle.SetOptStat(0)

def root_env():
    buildpath = os.environ["SBN_LIB_DIR"]
    if not buildpath:
        print "ERROR: SBNDDAQ_ANALYSIS_BUILD_PATH not set"
        sys.exit()
    ROOT.gROOT.ProcessLine(".L " + buildpath + "/libsbnanalysis_Event.so")
    ROOT.gROOT.ProcessLine(".L " + buildpath + "/libsbnanalysis_PandoraTesting_classes.so")
    ROOT.gROOT.ProcessLine(".L " + buildpath + "/libsbnanalysis_SBNOsc_classes.so")
    ROOT.gROOT.ProcessLine(".L " + buildpath + "/libsbnanalysis_SBNOscReco_classes.so")

def wait(args):
    if args.wait:
        raw_input("Press Enter to continue...")

def write(args, canvas):
    if args.output:
        canvas.SaveAs(args.output)

def with_input_args(parser):
    parser.add_argument("-i", "--input", required=True)
    return parser

def with_display_args(parser):
    parser.add_argument("-w", "--wait", action="store_true")
    parser.add_argument("-o", "--output", default=None)
    return parser

def with_io_args(parser):
    parser = with_input_args(parser)
    parser = with_display_args(parser)
    return parser

def with_graphsize_args(parser):
    parser.add_argument("-ym", "--y_min", type=float, default=None)
    parser.add_argument("-yh", "--y_max", type=float, default=None)
    parser.add_argument("-rl", "--range_lo", type=float, default=None)
    parser.add_argument("-rh", "--range_hi", type=float, default=None)
    return parser

def with_histosize_args(parser):
    parser.add_argument("-r", "--rebin", type=int, default=1)
    return with_graphsize_args(parser)

def resize_graph(args, hist):
    if args.range_lo is not None and args.range_hi is not None: 
        hist.GetXaxis().SetLimits(args.range_lo, args.range_hi)
    if args.y_min is not None and args.y_max is not None: 
        hist.GetYaxis().SetRangeUser(args.y_min, args.y_max)

def resize_histo(args, hist):
    if args.range_lo is not None and args.range_hi is not None: 
        range_lo_ind = None
        for i in range(1, hist.GetNbinsX()+1):
            if range_lo_ind is None and hist.GetBinLowEdge(i) > args.range_lo:
                range_lo_ind = max(0, i-2)
            if hist.GetBinLowEdge(i) >= args.range_hi:
                range_hi_ind = i
                break
        else: 
            range_hi_ind = hist.GetNbinsX()+1
        assert(range_lo_ind < range_hi_ind)

        x_axis = [hist.GetXaxis().GetBinLowEdge(i) for i in range(1,hist.GetXaxis().GetNbins()+1)] + [hist.GetXaxis().GetBinUpEdge(hist.GetXaxis().GetNbins())]
        new_x_axis = array('d', [x_axis[i] for i in range(range_lo_ind, range_hi_ind)])
        new_hist = ROOT.TH1D(hist.GetName() + " resized", hist.GetTitle(), len(new_x_axis)-1, new_x_axis)
        for i in range(range_lo_ind+1, range_hi_ind+1):
            new_hist.SetBinContent(i - range_lo_ind, hist.GetBinContent(i))
        hist = new_hist

    if args.rebin != 1:
        hist.Rebin(args.rebin) 
    return hist

def with_histostyle_args(parser):
    parser.add_argument("-xl", "--xlabel", default=None)
    parser.add_argument("-yl", "--ylabel", default=None)
    return parser

def style(args, hist):
    hist.GetYaxis().SetTitleSize(20)
    hist.GetYaxis().SetTitleFont(43)
    hist.GetYaxis().SetLabelFont(43)
    hist.GetYaxis().SetLabelSize(20)
    hist.GetYaxis().CenterTitle()

    hist.GetXaxis().SetTitleSize(20)
    hist.GetXaxis().SetTitleFont(43)
    hist.GetXaxis().SetLabelFont(43)
    hist.GetXaxis().SetLabelSize(20)
    hist.GetXaxis().CenterTitle()

    if args.xlabel: hist.GetXaxis().SetTitle(args.xlabel)
    if args.ylabel: hist.GetYaxis().SetTitle(args.ylabel)

def colors(index):
    colors = [ROOT.kRed, ROOT.kGreen]
    return colors[index]

def fillcolors(index):
    colors = [ROOT.kGreen + 1, ROOT.kCyan -9 , ROOT.kBlue+1,  ROOT.kViolet, ROOT.kRed +2, ROOT.kYellow+3, ROOT.kBlack]
    return colors[index]

def comma_separated(inp):
    return inp.split(",")

def legend_position(inp):
    if inp == "ur":
        return [0.75,0.75,0.95,0.95]
    elif inp == "ul":
        return [0.35,0.75,0.15,0.95]
    elif inp == "um":
        return [0.4, 0.75, 0.6, 0.95]
    else:
        return [float(x) for x in inp.split(",")][:4]

def histo_list(inp):
    inp = inp.split(",")
    in_parens = False
    out = []
    for name in inp:
        if name.startswith("("):
            assert(not in_parens)
            in_parens = True
            name = name.lstrip("(")
            out.append([name])
        elif name.endswith(")"):
            assert(in_parens)
            in_parens = False
            name = name.rstrip(")")
            out[-1].append(name)
        elif in_parens:
            out[-1].append(name)
        else:
            out.append([name])
    return out

def validate_hists(names, hists):
    for nn,hh in zip(names, hists):
        if isinstance(hh, list):
            for n,h in zip(nn,hh):
                if not h:
                     raise Exception("Error: invalid histogram with name (%s)" % n)
                    # raise Exception("Error: invalid histogram (%s) with name (%s)" % (h, n))
        else:
            if not h:
                raise Exception("Error: invalid histogram (%s) with name (%s)" % (hh, nn))


