cmd_/home/jiwook/data/driver/Platform_LED_driver/Module.symvers := sed 's/\.ko$$/\.o/' /home/jiwook/data/driver/Platform_LED_driver/modules.order | scripts/mod/modpost -m -a  -o /home/jiwook/data/driver/Platform_LED_driver/Module.symvers -e -i Module.symvers   -T -
