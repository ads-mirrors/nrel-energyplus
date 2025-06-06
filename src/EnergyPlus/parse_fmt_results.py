import pandas as pd
from io import StringIO
import subprocess
from pathlib import Path

EP_CLI = Path('~/Software/Others/EnergyPlus-build-release2/Products/test_fmt').expanduser()
assert EP_CLI.is_file()

r = subprocess.check_output([str(EP_CLI), '--debug-fmt'], encoding='utf-8', universal_newlines=True)
df_z_ori = pd.read_csv(StringIO(r), dtype='object')
df_z_ori = df_z_ori.set_index(['Number', 'Precision'])
df_z_ori.columns = pd.MultiIndex.from_tuples(df_z_ori.columns.str.split('_').tolist(), names=['fmt_type', 'item'])
df_z_ori = df_z_ori.swaplevel(0, 1, axis=1)['value']
df_z_ori.to_csv('fmt.csv')
