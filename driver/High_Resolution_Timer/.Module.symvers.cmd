cmd_/home/jiwook/data/driver/High_Resolution_Timer/Module.symvers := sed 's/\.ko$$/\.o/' /home/jiwook/data/driver/High_Resolution_Timer/modules.order | scripts/mod/modpost -m -a  -o /home/jiwook/data/driver/High_Resolution_Timer/Module.symvers -e -i Module.symvers   -T -
