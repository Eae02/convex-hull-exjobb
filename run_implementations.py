from typing import List
import subprocess
import sys

import testlib

def run_implementations(implementations : List[str]):
    test_cases = [("gencirc.bin",[(100,20),(200,20),(500,20),(1000,20),(2000,20),(5000,20),(10000,20),(20000,10),(50000,10),(100000,10),(200000,5),(500000,5),(1000000,5),(2000000,5),(5000000,5),(10000000,5)])] #test_generator,[n,num_iterations]
    test_cases.extend([("gensquare.bin",[(100,20),(200,20),(500,20),(1000,20),(2000,20),(5000,20),(10000,20),(20000,10),(50000,10),(100000,10),(200000,5),(500000,5),(1000000,5),(2000000,5),(5000000,5),(10000000,5)])])
    test_cases.extend([("gendisk.bin",[(100,20),(200,20),(500,20),(1000,20),(2000,20),(5000,20),(10000,20),(20000,10),(50000,10),(100000,10),(200000,5),(500000,5),(1000000,5),(2000000,5),(5000000,5),(10000000,5)])])
    results = [] # Table of results containing: implementation_name, test_generator, seed, n, compute_time
    subprocess.run(['mkdir', 'results'])
    test_case_path = '.testcases/tmp.in'
    with open('results/times.csv','w') as f:
        f.write("implementation_name,test_generator,seed,n,compute_time(ms)\n")

        seed = 1000
        for test_generator, cases in test_cases:
            timed_out = set() #Keep track of implementations that time out and don't run them again.
            for n, num_iterations in cases:
                print(f'Running tests generated by {test_generator} with size {n} measuring a total of {num_iterations} times')
                for iter in range(num_iterations):
                    seed +=1
                    with open(test_case_path,'w') as test_case_file: 
                        bash_command = [f'./testtools/{test_generator}', f'seed={seed}', f'n={n}']
                        subprocess.run(bash_command, stdout = test_case_file)

                    for implementation_name in implementations:
                        if implementation_name in timed_out:
                            continue
                        try:
                            time_limit = max(3, 2*n/1000000) #3 seconds or 2 us per input point
                            compute_time = testlib.run(test_case_path, implementation_name, timeout = time_limit) 
                        except subprocess.TimeoutExpired:
                            timed_out.add(implementation_name)
                            continue
                        #print(test_generator,n,seed,implementation_name,compute_time)
                        results.append((implementation_name, test_generator, seed, n, compute_time))
                        f.write(f'{implementation_name},{test_generator},{seed},{n},{compute_time}\n')
                    if n > 100000:
                        print(f'Iteration {iter} done')






def main():
    # Uncomment to pick which implementations to run on

    # All non-jarvis sequential implementations
    # implementations = ["cgal_akl_toussaint", "cgal_bykat", "cgal_eddy", "cgal_graham", "chan_plain", "chan_idea1", "chan_idea2", "chan_idea12", "chan_idea3", "chan_refined", "dc_preparata_hong", "dc_preparata_hong_rewrite", "impl1", "merge_hull", "merge_hull_chan_trick", "merge_hull_reduce_copy", "qh_rec", "qhp_seq"]
    
    # Some sequential implementations
    # implementations = ["cgal_akl_toussaint", "cgal_graham", "chan_plain", "chan_refined", "merge_hull_reduce_copy", "dc_preparata_hong_rewrite", "impl1", "qh_rec"]

    # Some sequential and parallel implementations
    # implementations = ["cgal_akl_toussaint", "cgal_graham", "chan_plain", "chan_refined", "merge_hull_reduce_copy", "dc_preparata_hong_rewrite", "impl1", "impl1_par", "qh_rec", "qh_recpar", "qh_avx"]

    # All interesting implementations for our thesis
    implementations = ["chan", "chan_refined", "dc_preparata_hong", "impl1", "mc", "merge_hull", "merge_hull_chan_trick", "ouellet", "qh_bf_nxp", "qh_bf_ss", "qh_bf_xp", "qh_hybrid_esx", "qh_hybrid_jw", "qh_hybrid_mc", "qh_rec_esx", "qh_rec_nxp", "qh_rec_ss", "qh_rec_xp", "qh_rec_xyp", "qh_soa"]
    implementations.extend(["cgal_akl_toussaint","cgal_bykat", "cgal_eddy", "cgal_graham", "cgal_jarvis", "qhull"])
    implementations.extend(["jarvis_wrap"])
    implementations.extend(["impl1_par", "qh_recpar_nxp", "qh_recpar_xp", "qhp_bf", "qhp_bf_nr", "qhp_bf_seq"])
    # implementations = ["qh_rec", "qh_rec_nxp", "qh_rec_esx", "qh_hybrid_esx:D1", "qh_hybrid_esx:D2", "qh_hybrid_esx:D3"]
    # implementations = ["qh_rec", "qh_bf"]

    # Comparisons with parallell implementations that run on POWER
    # implementations = ["impl1", "impl1_par", "qh_rec", "qh_recpar", "qhp", "qhp_nr", "qhp_seq"]

    # A subset of sequential implementations that run on POWER
    # implementations = ["chan_plain", "chan_refined", "qh_rec", "merge_hull_reduce_copy", "impl1", "dc_preparata_hong_rewrite"]

    # Chan variants
    # implementations = ["chan_plain", "chan_idea12", "chan_idea3", "chan_refined", "qh_rec", "merge_hull_reduce_copy", "merge_hull_chan_trick", "impl1"]

    # Test of Ouellets algorithm
    # implementations = ["chan_plain", "chan_refined", "qh_rec", "merge_hull_reduce_copy", "impl1", "ouellet"]
    
    # implementations = ["qh_rec", "qh_bf:A", "qh_bf:N"]

    run_implementations(implementations)
    if len(sys.argv)>1 and sys.argv[1] == "plot":
        import plot_results
        plot_results.plot_results('results/times.csv')

if __name__ == "__main__":
    main()
