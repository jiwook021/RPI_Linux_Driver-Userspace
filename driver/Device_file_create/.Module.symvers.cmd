cmd_/home/jiwook/data/driver/Device_file_create/Module.symvers := sed 's/\.ko$$/\.o/' /home/jiwook/data/driver/Device_file_create/modules.order | scripts/mod/modpost -m -a  -o /home/jiwook/data/driver/Device_file_create/Module.symvers -e -i Module.symvers   -T -
