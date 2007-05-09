#ifndef MMPREFS_H
#define MMPREFS_H

class MMPrefs
{
public:
	void Load();
	void Save() const;

	struct CPUPrefs
	{
		double k;
		double damper;
		bool use_cpu_count;
	};

	const CPUPrefs& GetCPUPrefs() const;

private:
	CPUPrefs cpuprf;
};

#endif//MMPREFS_H
